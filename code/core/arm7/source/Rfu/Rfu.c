/* gameplaySP
 *
 * Copyright (C) 2023 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"
#include <libtwl/spi/spiPmic.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/sys/swi.h>
#include <string.h>
#include "Wifi/wifi.h"
#include "GbaSio.h"
#include "GbaIoRegOffsets.h"
#include "RfuWifi.h"
#include "Rfu.h"

#define logFromC(...)

// #define RFU_DEBUG

// Debug print logic:
#ifdef RFU_DEBUG
  #define RFU_DEBUG_LOG(...) logFromC(__VA_ARGS__)
#else
  #define RFU_DEBUG_LOG(...)
#endif

// Config knobs, update with care.
#define MAX_RFU_PEERS                 4    // Do not allow more than 4 peers.

#define RFU_DEF_TIMEOUT               0   // Expressed as frames (~533ms)
#define RFU_DEF_RTXMAX                4   // Up to 4 transmissions per send

// The following commands, names and bit fields are not a 100% known.
// Most of them is guessed via reverse engineering the adapter and/or
// games that use it. Please take it with a grain of salt.
// You might wanna check:
// https://github.com/afska/gba-link-connection/
// https://blog.kuiper.dev/gba-wireless-adapter

#define RFU_CONN_INPROGRESS  0x01000000     // Connection ongoing

#define RFU_CMD_INIT1        0x10   // Dummy after-init command
#define RFU_CMD_INIT2        0x3d   // Dummy after-init command
#define RFU_CMD_SYSCFG       0x17   // System configuration (comms. config)

#define RFU_CMD_LINKPWR      0x11   // Link/RF strength (0 to 0x16/0xFF)

// These are not really well documented.
#define RFU_CMD_SYSVER       0x12   // Return some 1 word with version info.
#define RFU_CMD_SYSSTAT      0x13   // System/Connection status.
#define RFU_CMD_SLOTSTAT     0x14   // Reads slot status information.
#define RFU_CMD_CFGSTAT      0x15   // Reads some sort of config state.

#define RFU_CMD_BCST_DATA    0x16   // Broadcast data setup

#define RFU_CMD_HOST_START   0x19   // Start broadcasting and accepting clients
#define RFU_CMD_HOST_ACCEPT  0x1a   // Poll for incoming connections
#define RFU_CMD_HOST_STOP    0x1b   // Stop broadcasting

#define RFU_CMD_BCRD_START   0x1c   // Broadcast read session start
#define RFU_CMD_BCRD_FETCH   0x1d   // Broadcast read (actual data read)
#define RFU_CMD_BCRD_STOP    0x1e   // Broadcast read session end

#define RFU_CMD_CONNECT      0x1f   // Connect to host
#define RFU_CMD_ISCONNECTED  0x20   // Check for conection status
#define RFU_CMD_CONCOMPL     0x21   // Complete connection?

#define RFU_CMD_SEND_DATA    0x24   // Sends a data packet
#define RFU_CMD_SEND_DATAW   0x25   // Sends a data packet and waits
#define RFU_CMD_RECV_DATA    0x26   // Receive (poll) some data
#define RFU_CMD_WAIT         0x27   // Wait (for response or timeout)
#define RFU_CMD_RTX_WAIT     0x37   // Wait after some retransmit?

#define RFU_CMD_DISCONNECT   0x30   // Disconnects clients

// RFU commands for slave mode (~command responses)
#define RFU_CMD_RESP_TIMEO   0x27   // Timeout!
#define RFU_CMD_RESP_DATA    0x28   // There's data available
#define RFU_CMD_RESP_DISC    0x29   // Some clients disconnected


// These are internal FSM states for the communication steps.
#define RFU_COMSTATE_RESET       0    // Just out of reset
#define RFU_COMSTATE_HANDSHAKE   1    // Performing initial nintendo handshake
#define RFU_COMSTATE_WAITCMD     2    // Idle, waiting for a command
#define RFU_COMSTATE_WAITDAT     3    // Waiting for follow up data
#define RFU_COMSTATE_RESPCMD     4    // RFU to device response (cmd)
#define RFU_COMSTATE_RESPDAT     5    // RFU to device response (N words)
#define RFU_COMSTATE_RESPERR     6    // Send back the error command
#define RFU_COMSTATE_RESPERR2    7    // Send back the error code
#define RFU_COMSTATE_WAITEVENT   8    // Waiting for event or timeout
#define RFU_COMSTATE_WAITRESP    9    // Responding a wait command

// These FSM states describe the adapter wifi state.
#define RFU_STATE_IDLE            0
#define RFU_STATE_HOST            1    // Hosting (with broadcast)
#define RFU_STATE_SHOST           2    // Hosting (without broadcast)
#define RFU_STATE_CLIENT          3    // Client, connected to a host


static u32 rfu_prev_data;
static u32 rfu_comstate, rfu_cnt, rfu_state;
static u32 rfu_buf[255];
static u8 rfu_cmd, rfu_plen;
static u32 rfu_timeout_frames, rfu_resp_timeout;
static u8 rfu_timeout, rfu_rtx_max;
static bool sExternalClockTransferStarted;

static struct {
  u32 buf[23];
  u8 blen;
} rfu_tx_buf;

static struct {
  u16 devid;         // Device ID assigned to the host
  u8 disc_flag;      // Disconnected devices flag.
  u8 userName[8];
  u8 gameName[16];
  struct {
    wifi_macaddr_t clientMacAddress;
    u16 associationId;       // Host-assigned device ID
    u32 datalen;   // Byte count of data waiting to be polled.
    u8  data[16];  // Data received from client.
  } clients[4];      // Connected clients IDs (zero means empty slot).
} rfu_host;

static struct {
  u16 devid;         // Device ID assigned to the client (by the host?)
  u16 clnum;         // Client number (0 to 3)
  wifi_macaddr_t parentBssid;
  // Store host received packets (up to 8!)
  u16 hblen;       // Bytes received from the host.
  u8 hdata[128];   // Data received from the host (accumulated).
} rfu_client;

typedef struct {
    u8 valid;
    u8 ttl;            // When it reaches zero, entry is invalidated
    wifi_macaddr_t parentBssid;
    rfu_beacon_data_t beaconData;
} t_client_broadcast;

static t_client_broadcast rfu_peer_bcst[MAX_RFU_PEERS];

static sio_shared_t* sSioSharedData;

void rfu_init(sio_shared_t* sharedData)
{
    sSioSharedData = sharedData;
    wifi_init();
    wifi_start();
}

// This is called whenever the game uses the GPIO (pin D) to perform a reset
// pin flip in the external reset pin. We reset the device to a known state.
void rfu_reset(void) {
  RFU_DEBUG_LOG("RFU reset!\n");
  // Reset FSMs to a known state
  rfu_prev_data = 0;
  rfu_cnt = 0;
  rfu_state = RFU_STATE_IDLE;
  rfu_comstate = RFU_COMSTATE_RESET;
  rfu_timeout_frames = 0;
  rfu_resp_timeout = 0;
  rfu_timeout = RFU_DEF_TIMEOUT;
  rfu_rtx_max = RFU_DEF_RTXMAX;
  sExternalClockTransferStarted = false;
  memset(&rfu_host, 0, sizeof(rfu_host));

  // Clear all the received broadcasts.
  memset(&rfu_peer_bcst, 0, sizeof(rfu_peer_bcst));

  sSioSharedData->si = false;
  sSioSharedData->sioToGbaData32 = 0;

  rfuw_stopParentMode();

  // Re-seed random gen.
//   rand_seed(time(NULL));
//   rand_seed(cpu_ticks);
}

static u16 new_devid(const wifi_macaddr_t* macAddress)
{
    u32 result;
    do
    {
        result = REG_WIFI_MSEQ16;
        result = (result * 397) ^ macAddress->address16[0];
        result = (result * 397) ^ macAddress->address16[1];
        result = (result * 397) ^ macAddress->address16[2];
        result = result & 0xFFFC;
    } while (result == 0);
    return (u16)result;
}

static void sendDisassociationToParent(void)
{
    logFromC("sendDisassociationToParent\n");
}

static void sendDisassociationToChild(int child)
{
    logFromC("sendDisassociationToChild\n");
}

// We have received a command in full (with a potential payload), process it
// and return the return command code (plus some payload too?).
static s32 rfu_process_command() {
  u32 i, j, cnt = 0;
  RFU_DEBUG_LOG("Processing command 0x%x\n", rfu_cmd);

  switch (rfu_cmd) {
  // These are not 100% supported, but they are OK as long as we ACK them.
  case RFU_CMD_INIT1:
  case RFU_CMD_INIT2:
    return 0;

  case RFU_CMD_SYSCFG:
    // Contains the slave timeout and retransmit count.
    rfu_timeout = rfu_buf[0];
    rfu_rtx_max = rfu_buf[0] >> 8;
    return 0;

  case RFU_CMD_SYSVER:
    rfu_buf[0] = 0x00830117;   // Likely some sort of firmware/hw version.
    return 1;

  case RFU_CMD_SYSSTAT:
    // WIP This is still not well described!
    if (rfu_state == RFU_STATE_SHOST || rfu_state == RFU_STATE_HOST)
    {
        rfu_buf[0] = 0x01000000 | rfu_host.devid;
    }
    else if (rfu_state == RFU_STATE_CLIENT)
    {
        rfu_buf[0] = 0x05000000 | rfu_client.devid | (0x10000 << rfu_client.clnum);
    }
    else
    {
        rfu_buf[0] = 0x00000000;
    }

    return 1;

  case RFU_CMD_SLOTSTAT:
    if (rfu_state == RFU_STATE_SHOST || rfu_state == RFU_STATE_HOST)
    {
        // Just a list of connected devices it seems
        u32 cnt = 0;
        rfu_buf[cnt++] = 0xFF;
        for (i = 0; i < 4; i++)
        {
            if (!rfu_host.clients[i].associationId)
            {
                rfu_buf[0] = i;
                break;
            }
        }
        for (i = 0; i < 4; i++)
        {
            if (rfu_host.clients[i].associationId)
            {
                rfu_buf[cnt++] = rfu_host.clients[i].associationId | (i << 16);
            }
        }
        return cnt;
    }
    return 0;

  case RFU_CMD_LINKPWR:
    // TODO: Return something better (ie. latency?)
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST)
      rfu_buf[0] = (rfu_host.clients[0].associationId ? 0x000000ff : 0) |
                   (rfu_host.clients[1].associationId ? 0x0000ff00 : 0) |
                   (rfu_host.clients[2].associationId ? 0x00ff0000 : 0) |
                   (rfu_host.clients[3].associationId ? 0xff000000 : 0);
    else if (rfu_state == RFU_STATE_CLIENT)
      rfu_buf[0] = 0xFF << (rfu_client.clnum * 8);
    else
      rfu_buf[0] = 0;
    return 1;

  // Process broadcast read sessions.
  // TODO return errors if outside of a session.
  case RFU_CMD_BCRD_START:
    rfuw_startSearchMode();
    return 0;   // Return an ACK immediately.

  case RFU_CMD_BCRD_STOP:
    rfuw_stopSearchMode();

  case RFU_CMD_BCRD_FETCH:
    // Return all broadcasts that we have received.
    for (i = 0, cnt = 0; i < MAX_RFU_PEERS; i++) {
      if (rfu_peer_bcst[i].valid)
      {
          memcpy(&rfu_buf[cnt], &rfu_peer_bcst[i].beaconData, sizeof(rfu_beacon_data_t));
          cnt += 7;
      }
    }
    return cnt;

  // Sets the broadcast data to use on Host mode
  case RFU_CMD_BCST_DATA:
    if (rfu_plen == 6)
    {
        memcpy(rfu_host.gameName, &rfu_buf[0], sizeof(rfu_host.gameName));
        memcpy(rfu_host.userName, &rfu_buf[4], sizeof(rfu_host.userName));
    }
    return 0;

  case RFU_CMD_HOST_START:
    if (rfu_state == RFU_STATE_CLIENT) {
      RFU_DEBUG_LOG("RFU error: Cannot start host, we are a client already\n");
      return -1;   // Return error if we are connected (state client)
    }

    if (rfu_state == RFU_STATE_IDLE) {
      // Generate a new ID in this case.
      rfu_host.devid = new_devid(&WIFI_RAM->firmData.wifiData.macAddress);
      RFU_DEBUG_LOG("Start hosting with device ID %02x\n", rfu_host.devid);
      rfu_state = RFU_STATE_HOST;    // Host mode active
      rfuw_startParentMode(rfu_host.devid, /*todo*/ 0, rfu_host.userName, rfu_host.gameName);
    }
    return 0;

  case RFU_CMD_HOST_STOP:
    if (rfu_state == RFU_STATE_IDLE)
      return -1;  // Return error if host mode is not active

    rfuw_stopSendParentBeacons();

    // If no clients are connected, stop host mode for real.
    for (i = 0; i < 4; i++) {
      if (rfu_host.clients[i].associationId) {
        // We found one valid client so far. Slient host it is.
        rfu_state = RFU_STATE_SHOST;
        return 0;
      }
    }
    rfuw_stopParentMode();
    rfu_state = RFU_STATE_IDLE;
    return 0;

  case RFU_CMD_HOST_ACCEPT:
    if (rfu_state == RFU_STATE_IDLE)
      return -1;  // Return error if host mode is not active

    // This is actually a "list connected devices", not actually accept().
    // Return a list of IDs and a client number (slot number).
    for (i = 0; i < 4; i++)
      if (rfu_host.clients[i].associationId)
        rfu_buf[cnt++] = rfu_host.clients[i].associationId | (i << 16);
    return cnt;

  case RFU_CMD_CONNECT:
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST)
      return -1;  // Return error if host mode is active

    // The game specified a device ID, find the host.
    {
      u16 reqid = rfu_buf[0] & 0xffff;
      for (i = 0, cnt = 0; i < MAX_RFU_PEERS; i++) {
        if (rfu_peer_bcst[i].valid &&
            rfu_peer_bcst[i].beaconData.parentId == reqid) {

          // Send a request to the host to connect
          rfu_client.parentBssid = rfu_peer_bcst[i].parentBssid;
          rfuw_startConnectToParent(&rfu_peer_bcst[i].parentBssid);
          return 0;
        }
      }
    }
    // If the ID cannot be found, return an error?
    return -1;

  case RFU_CMD_ISCONNECTED:
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST)
      return -1;  // Return error if host mode is active

    rfu_buf[0] = !rfu_client.devid ? RFU_CONN_INPROGRESS :
                 (rfu_client.devid | (rfu_client.clnum << 16));
    return 1;

  case RFU_CMD_CONCOMPL:
    // Seems that this is also called even when no connection happened!
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST)
      return -1;

    // Not sure what to return here, when the connection timedout/was nacked.
    if (!rfu_client.devid)
      return 0;

    rfu_buf[0] = rfu_client.devid;
    return 1;

  case RFU_CMD_SEND_DATAW:
  case RFU_CMD_SEND_DATA:
    if (!rfu_plen)
      return 0;

    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST) {
      // Read data to be sent into the TX buffer
      rfu_tx_buf.blen = rfu_buf[0] & 0x7F;
      memcpy(rfu_tx_buf.buf, &rfu_buf[1], (rfu_plen - 1)*sizeof(u32));
    }
    else if (rfu_state == RFU_STATE_CLIENT) {
      // Same as above, but the header encoding is funny
      rfu_tx_buf.blen = (rfu_buf[0] >> (8 + rfu_client.clnum * 5)) & 0x1F;
      memcpy(rfu_tx_buf.buf, &rfu_buf[1], (rfu_plen - 1)*sizeof(u32));
    }

    /* fallthrough */
  case RFU_CMD_RTX_WAIT:
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST) {
      // Host sends a package to all clients.
      RFU_DEBUG_LOG("Host sending %d bytes / %d words to clients\n",
                    rfu_tx_buf.blen, rfu_plen - 1);
      if (rfu_tx_buf.blen <= 90)
      {
          u32 childMask = 0;
          u32 childCount = 0;
          for (u32 i = 0; i < 4; i++)
          {
              if (rfu_host.clients[i].associationId)
              {
                  childMask |= (1 << (i + 1));
                  childCount++;
              }
          }
          rfuw_sendMultiPollParentCmd(rfu_tx_buf.buf, rfu_tx_buf.blen, childMask, childCount);
      }
    }
    else if (rfu_state == RFU_STATE_CLIENT) {
      // Schedule data to be sent
      RFU_DEBUG_LOG("Client sending %d bytes / %d words to host\n",
                    rfu_tx_buf.blen, rfu_plen - 1);
      if (rfu_tx_buf.blen <= 16) {
        rfuw_setMultiPollChildReplyData(&rfu_client.parentBssid, rfu_tx_buf.buf, rfu_tx_buf.blen);
      }
    }
    else
      return -1;   // We are not connected nor a host
    break;

  case RFU_CMD_RECV_DATA:
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST) {
      // Receive data from clients
      u32 cnt = 0;
      rfu_buf[cnt++] = 0;   // Header contains byte counts as a bitfield.
      for (i = 0; i < 4; i++) {
        u32 dlen = rfu_host.clients[i].datalen;
        if (rfu_host.clients[i].associationId && dlen != 0) {
          RFU_DEBUG_LOG("Host reads data from client buffer (%d bytes)\n", dlen);
          // Copy data into words into the RFU buffer.
          for (j = 0; j < (dlen + 3) / 4; j++)
            rfu_buf[cnt++] = *(u32*)&rfu_host.clients[i].data[j*4];
          // Update byte count header for this client
          rfu_buf[0] |= dlen << (8 + i * 5);
          rfu_host.clients[i].datalen = 0;
        }
      }
      return cnt;
    }
    else if (rfu_state == RFU_STATE_CLIENT) {
      u32 cnt = 0;
      u32 dlen = rfu_client.hblen;
      RFU_DEBUG_LOG("Client reads data from host buffer (%d bytes)\n", dlen);
      rfu_buf[cnt++] = dlen;   // Header contains byte count.
      for (j = 0; j < (dlen + 3) / 4; j++)
        rfu_buf[cnt++] = *(u32*)&rfu_client.hdata[j*4];
      rfu_client.hblen = 0;
      return cnt;
    }
    break;

  case RFU_CMD_WAIT:
    // Do nothing, return no data (just ack).
    // The handler will deal with clock reversing.
    return 0;

  case RFU_CMD_DISCONNECT:
    if (rfu_state == RFU_STATE_CLIENT) {
      // Assuming self-disconnect?
      sendDisassociationToParent();
      rfu_state = RFU_STATE_IDLE;
    } else if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST) {
      // We are disconnecing some client(s).
      for (i = 0; i < 4; i++)
        if (rfu_buf[0] & (1 << i)) {
          // Send disconnect notice, clear slot!
          sendDisassociationToChild(i);
          memset(&rfu_host.clients[i], 0, sizeof(rfu_host.clients[i]));
        }
    }
    return 0;

  default:
    RFU_DEBUG_LOG("Unknown RFU command %02x\n", rfu_cmd);
  };

  return 0;
}

// Returns true if a Wait event can finish due to new data being available.
static bool rfu_data_avail()
{
  if (rfu_state == RFU_STATE_CLIENT)
  {
    if (rfu_client.hblen != 0)
      return true;
  }
  else if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST)
  {
      // Returns true if any data from any client is available.
      for (u32 i = 0; i < 4; i++)
      {
          if (rfu_host.clients[i].associationId && !rfu_host.clients[i].datalen)
          {
              return false;
          }
      }
      return true;
  }
  return false;
}

static void checkEvent(void)
{
    if (rfu_comstate == RFU_COMSTATE_WAITEVENT)
    {
        if (rfu_data_avail())
        {
            rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_DATA;
            rfu_buf[1] = 0x80000000;
            rfu_cnt = 0;
            rfu_plen = 2;
            rfu_comstate = RFU_COMSTATE_WAITRESP;
        }
    }
    if (rfu_comstate == RFU_COMSTATE_WAITRESP && sExternalClockTransferStarted &&
        !sSioSharedData->si && !sSioSharedData->so)
    {
        sExternalClockTransferStarted = false;
        sSioSharedData->sioToGbaData32 = rfu_buf[rfu_cnt++];
        if (rfu_cnt == rfu_plen)
        {
            rfu_comstate = RFU_COMSTATE_WAITCMD;
        }
        swi_waitByLoop(256);
        sSioSharedData->irqRequest = true;
        ipc_triggerArm7Irq();
    }
}

#define HANDSHAKE_START   0x494EB6B1
#define HANDSHAKE_END     0xB0BB8001

const u32 handshake_fsm[][2] =
{
    {0xFFFF494E,0x494EB6B1},
    {0xB6B1494E,0x544EB6B1},
    {0xB6B1544E,0x544EABB1},
    {0xABB1544E,0x4E45ABB1},
    {0xABB14E45,0x4E45B1BA},
    {0xB1BA4E45,0x4F44B1BA},
    {0xB1BA4F44,0x4F44B0BB},
    {0xB0BB4F44,0x8001B0BB},
};

// Processes a received value from the GBA, returns the value to store
// in the slave register, to be sent back in the next SPI transfer.
u32 rfu_transfer(u32 sent_value)
{
  u32 i;
  switch (rfu_comstate)
  {
  case RFU_COMSTATE_RESET:
    // Start of sequence (check the low 16 bits)
    if ((sent_value & 0xFFFF) == 0x494E)
    {
      rfu_comstate = RFU_COMSTATE_HANDSHAKE;
      return HANDSHAKE_START;
    }
    break;

  case RFU_COMSTATE_HANDSHAKE:
    // Check for the last step of the sequence
    if (sent_value == HANDSHAKE_END)
    {
      rfu_comstate = RFU_COMSTATE_WAITCMD;
      return 0x80000000;
    }

    for (i = 0; i < sizeof(handshake_fsm) / sizeof(handshake_fsm[0]); i++)
      if (sent_value == handshake_fsm[i][0])
        return handshake_fsm[i][1];
    break;
  case RFU_COMSTATE_WAITCMD:
    // Wait for a new command, verify its header.
    if ((sent_value >> 16) == 0x9966)
    {
      rfu_plen = (u8)(sent_value >> 8);
      rfu_cmd = (u8)(sent_value);
      rfu_cnt = 0;
      if (rfu_plen)
      {
        rfu_comstate = RFU_COMSTATE_WAITDAT;
        return 0x80000000; // Need to accept more data
      }
      else
      {
        // Returns error code or response length
        s32 ret = rfu_process_command();
        if (ret < 0)
        {
          rfu_comstate = RFU_COMSTATE_RESPERR;
          rfu_cmd = (u32)(-ret); // Err code
          return 0x996601ee;     // Return error command
        }
        else
        {
            rfu_plen = (u32)ret;
            if (rfu_cmd == RFU_CMD_WAIT || rfu_cmd == RFU_CMD_RTX_WAIT || rfu_cmd == RFU_CMD_SEND_DATAW)
            {
                rfu_comstate = RFU_COMSTATE_WAITEVENT;
                rfu_timeout_frames = rfu_timeout;
                // rfu_resp_timeout = 
            }
            else
            {
                rfu_comstate = rfu_plen ? RFU_COMSTATE_RESPDAT : RFU_COMSTATE_WAITCMD;
            }
            return 0x99660080 | rfu_cmd | (rfu_plen << 8);
        }
      }
    }
    return 0x80000000;

  case RFU_COMSTATE_WAITDAT:
    rfu_buf[rfu_cnt++] = sent_value;
    if (rfu_cnt == rfu_plen)
    {
      s32 ret = rfu_process_command();
      if (ret < 0)
      {
        rfu_comstate = RFU_COMSTATE_RESPERR;
        rfu_cmd = (u32)(-ret); // Err code
        return 0x996601ee;     // Return error command
      }
      else
      {
        rfu_plen = (u32)ret;
        if (rfu_cmd == RFU_CMD_WAIT || rfu_cmd == RFU_CMD_RTX_WAIT || rfu_cmd == RFU_CMD_SEND_DATAW)
        {
            rfu_comstate = RFU_COMSTATE_WAITEVENT;
            rfu_timeout_frames = rfu_timeout;
        }
        else
        {
            rfu_comstate = rfu_plen ? RFU_COMSTATE_RESPDAT : RFU_COMSTATE_WAITCMD;
        }
        return 0x99660080 | rfu_cmd | (rfu_plen << 8);
      }
    }
    return 0x80000000;

  case RFU_COMSTATE_RESPERR:
    rfu_comstate = RFU_COMSTATE_WAITCMD;
    return rfu_cmd; // Some error code, not understood yet.

  case RFU_COMSTATE_RESPDAT:
  {
    u32 retval = rfu_buf[rfu_cnt++];
    if (rfu_cnt == rfu_plen)
      rfu_comstate = RFU_COMSTATE_WAITCMD;
    return retval;
  }
  }

  return 0;
}

void rfu_doTransfer(bool internalClock)
{
    if (!internalClock)
    {
        if (rfu_comstate == RFU_COMSTATE_WAITEVENT || rfu_comstate == RFU_COMSTATE_WAITRESP)
        {
            sExternalClockTransferStarted = true;
            checkEvent();
        }
    }
    else
    {
        u32 data = sSioSharedData->gbaToSioData32;
        swi_waitByLoop(256);
        sSioSharedData->sioToGbaData32 = rfu_transfer(data);//sSioSharedData->gbaToSioData32);
        sSioSharedData->irqRequest = true;
        ipc_triggerArm7Irq();
    }
}

void rfu_soChanged(void)
{
    checkEvent();
}

// Gets called every frame for basic device updates.
void rfu_frame_update(void)
{
    // If device is in reset state, do nothing really.
    if (rfu_comstate != RFU_COMSTATE_RESET)
    {
        u32 irq = rtos_disableIrqs();
        // Account for peer expiration.
        for (u32 i = 0; i < MAX_RFU_PEERS; i++)
        {
            if (rfu_peer_bcst[i].valid)
            {
                if (--rfu_peer_bcst[i].ttl == 0)
                {
                    rfu_peer_bcst[i].valid = 0;
                }
            }
        }

        if (rfu_comstate == RFU_COMSTATE_WAITEVENT)
        {
            if (rfu_timeout_frames != 0 && --rfu_timeout_frames == 0)
            {
                // We ran out of time, just return an "error" code
                rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_TIMEO;
                rfu_buf[1] = 0x80000000;
                rfu_cnt = 0;
                rfu_plen = 2;
                rfu_comstate = RFU_COMSTATE_WAITRESP;
            }
        }
        checkEvent();
        rtos_restoreIrqs(irq);
    }
}

void rfu_notifyBeaconReceived(const rfu_beacon_data_t* beaconData, const wifi_macaddr_t* parentBssid)
{
    for (u32 i = 0; i < MAX_RFU_PEERS; i++)
    {
        if (rfu_peer_bcst[i].valid && rfu_peer_bcst[i].beaconData.parentId == beaconData->parentId)
        {
            rfu_peer_bcst[i].ttl = 0xff;
            rfu_peer_bcst[i].beaconData = *beaconData;
            return;
        }
    }
    // Find an empty slot to add this new broadcast.
    for (u32 i = 0; i < MAX_RFU_PEERS; i++)
    {
        if (!rfu_peer_bcst[i].valid)
        {
            rfu_peer_bcst[i].ttl = 0xff;
            rfu_peer_bcst[i].valid = 1;
            rfu_peer_bcst[i].parentBssid = *parentBssid;
            rfu_peer_bcst[i].beaconData = *beaconData;
            return;
        }
    }
}

void rfu_notifyAssociationRequestReceived(const wifi_macaddr_t* childMacAddress)
{
    RFU_DEBUG_LOG("Received Conn Req\n");
    if (rfu_state == RFU_STATE_HOST)
    {
        // Ensure this client is not already connected!
        for (u32 i = 0; i < 4; i++)
        {
            if (rfu_host.clients[i].associationId &&
                rfu_host.clients[i].clientMacAddress.address16[0] == childMacAddress->address16[0] &&
                rfu_host.clients[i].clientMacAddress.address16[1] == childMacAddress->address16[1] &&
                rfu_host.clients[i].clientMacAddress.address16[2] == childMacAddress->address16[2])
            {
                RFU_DEBUG_LOG("Connection request ignored: already connected!\n");
                return;
            }
        }
        // Find an empty connection slot
        for (u32 i = 0; i < 4; i++)
        {
            if (!rfu_host.clients[i].associationId)
            {
                u16 newid = new_devid(childMacAddress) | i;
                rfu_host.clients[i].associationId = newid;
                rfu_host.clients[i].clientMacAddress = *childMacAddress;
                RFU_DEBUG_LOG("Connected client: assigned new devID %4X\n", newid);

                // Respond with ACK
                rfuw_sendAssociationResponseToChild(childMacAddress, newid, WIFI_ASSOCIATION_RESPONSE_STATUS_SUCCESS);
                return;
            }
        }
        // Not enough slots, NACK
        rfuw_sendAssociationResponseToChild(childMacAddress, 0, WIFI_ASSOCIATION_RESPONSE_STATUS_FULL);
    }
    else
    {
        // TODO: Return connection error
        RFU_DEBUG_LOG("Conn Req ignored, device is not in Host mode\n");
    }
}

void rfu_notifyAssociationResponseReceived(u16 associationId, u16 status)
{
    if (status == WIFI_ASSOCIATION_RESPONSE_STATUS_SUCCESS)
    {
        RFU_DEBUG_LOG("Received connection ACK");
        // Only ok if we are not connected (not hosting)
        if (rfu_state == RFU_STATE_IDLE)
        {
            // Clear state and install device ID and slot number.
            wifi_macaddr_t parentBssid = rfu_client.parentBssid;
            memset(&rfu_client, 0, sizeof(rfu_client));
            rfu_client.parentBssid = parentBssid;
            rfu_client.devid = associationId;
            rfu_client.clnum = associationId & 3;
            rfu_state = RFU_STATE_CLIENT;
        }
    }
    else
    {
        // TODO: How do we handle a connection reject?
        // For now do nothing and timeout will handle it.
        RFU_DEBUG_LOG("Received CONN NACK, no action taken (unimplemented)\n");
    }
}

void rfu_notifyParentDataReceived(const u8* data, u32 dataLength)
{
    // Only possible if we are a client
    if (rfu_state == RFU_STATE_CLIENT)
    {
        // Receive data from the host. Queue que packet if possible
        if (!rfu_client.hblen)
        {
            memcpy(&rfu_client.hdata, data, dataLength);
            rfu_client.hblen = dataLength;
            RFU_DEBUG_LOG("Received host packet (%d bytes)\n",
                          dataLength);
            checkEvent();
            return;
        }
        RFU_DEBUG_LOG("Client dropped a host packet\n");
    }
}

void rfu_notifyChildDataReceived(u32 childId, const u8* data, u32 dataLength)
{
    // Only available when we are hosting
    if (rfu_state == RFU_STATE_HOST || rfu_state == RFU_STATE_SHOST)
    {
        if (rfu_host.clients[childId].associationId)
        {
            if (!rfu_host.clients[childId].datalen)
            {
                memcpy(rfu_host.clients[childId].data, data, dataLength);
                rfu_host.clients[childId].datalen = dataLength;
                RFU_DEBUG_LOG("Received client packet (%d bytes)\n",
                              dataLength);
                return;
            }
            RFU_DEBUG_LOG("Host dropped a client packet\n");
        }
    }
}

void rfu_notifyMpEnd(u32 failChildMask)
{
    if (rfu_comstate == RFU_COMSTATE_WAITEVENT)
    {
        if (failChildMask == 0)
        {
            rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_DATA;
            rfu_buf[1] = 0x80000000;
            rfu_plen = 2;
        }
        else
        {
            u32 okChildMask = failChildMask ^ 0xF; // TODO: just using 4 slots for now
            rfu_buf[0] = 0x99660000 | RFU_CMD_RESP_DATA | (1 << 8);
            rfu_buf[1] = (0xF << 8) | okChildMask;
            rfu_buf[2] = 0x80000000;
            rfu_plen = 3;
        }
        rfu_cnt = 0;
        rfu_comstate = RFU_COMSTATE_WAITRESP;
    }
    checkEvent();
}
