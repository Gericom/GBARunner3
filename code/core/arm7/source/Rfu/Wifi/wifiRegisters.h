#pragma once

#define REG_WIFI_DCF_LAST_ADRS          (*(volatile wifi_macaddr_t*)0x4805F70)
#define REG_WIFI_MP_LAST_SEQCTRL        (*(vu16*)0x4805F7E)

#define REG_WIFI_VERSION                (*(vu16*)0x4808000)
#define REG_WIFI_MAC_CMD                (*(vu16*)0x4808004)
#define REG_WIFI_MAC_CONFIG             (*(vu16*)0x4808006)
#define REG_WIFI_TX_CONFIG              (*(vu16*)0x4808008)
#define REG_WIFI_RX_CONFIG              (*(vu16*)0x480800A)
#define REG_WIFI_ISR                    (*(vu16*)0x4808010)
#define REG_WIFI_IMR                    (*(vu16*)0x4808012)

#define REG_WIFI_MAC_ADRS               (*(volatile wifi_macaddr_t*)0x4808018)
#define REG_WIFI_BSSID                  (*(volatile wifi_macaddr_t*)0x4808020)
#define REG_WIFI_KSID                   (*(vu16*)0x4808028)
#define REG_WIFI_AID                    (*(vu16*)0x480802A)
#define REG_WIFI_RETRY_LIMIT            (*(vu16*)0x480802C)
#define REG_WIFI_BASIC_RATE_SET         (*(vu16*)0x480802E)
#define REG_WIFI_MDP_CONFIG             (*(vu16*)0x4808030)
#define REG_WIFI_WEP_CONFIG             (*(vu16*)0x4808032)
#define REG_WIFI_MOD_CTRL               (*(vu16*)0x4808034)
#define REG_WIFI_SHUTDOWN               (*(vu16*)0x4808036)
#define REG_WIFI_WAKEUP_CTRL            (*(vu16*)0x4808038)
#define REG_WIFI_SET_POWER              (*(vu16*)0x480803C)
#define REG_WIFI_SET_FORCE_POWER        (*(vu16*)0x4808040)
#define REG_WIFI_MSEQ16                 (*(vu16*)0x4808044)
#define REG_WIFI_MP_POWER_SEQ           (*(vu16*)0x4808048)

#define REG_WIFI_RXBUF_STR              (*(vu16*)0x4808050)
#define REG_WIFI_RXBUF_END              (*(vu16*)0x4808052)
#define REG_WIFI_RXBUF_CUR              (*(vu16*)0x4808054)
#define REG_WIFI_RXBUF_WCUR             (*(vu16*)0x4808056)
#define REG_WIFI_RDMA_STR               (*(vu16*)0x4808058)
#define REG_WIFI_RXBUF_BNR              (*(vu16*)0x480805A)
#define REG_WIFI_RDMA_CNT               (*(vu16*)0x480805C)
#define REG_WIFI_RDMA_PORT              (*(vu16*)0x4808060)
#define REG_WIFI_RDMA_JUMP              (*(vu16*)0x4808062)
#define REG_WIFI_RDMA_JUMP_CNT          (*(vu16*)0x4808064)
#define REG_WIFI_WDMA_STR               (*(vu16*)0x4808068)
#define REG_WIFI_WDMA_CNT               (*(vu16*)0x480806C)
#define REG_WIFI_WDMA_PORT              (*(vu16*)0x4808070)
#define REG_WIFI_WDMA_JUMP              (*(vu16*)0x4808074)
#define REG_WIFI_WDMA_JUMP_CNT          (*(vu16*)0x4808076)
#define REG_WIFI_DMA_CTRL               (*(vu16*)0x4808078)

#define REG_WIFI_BEACON_ADRS            (*(vu16*)0x4808080)
#define REG_WIFI_BCN_PARAM_OFFSET       (*(vu16*)0x4808084)
#define REG_WIFI_BCN_PARAM              (*(vu16*)0x4808088)
#define REG_WIFI_BEACON_PERIOD          (*(vu16*)0x480808C)
#define REG_WIFI_TIM_COUNT              (*(vu16*)0x480808E)
#define REG_WIFI_MP_ADR                 (*(vu16*)0x4808090)
#define REG_WIFI_KEYIN_ADR              (*(vu16*)0x4808094)
#define REG_WIFI_KEYOUT_ADR             (*(vu16*)0x4808098)
#define REG_WIFI_SLOT_CONFIG            (*(vu16*)0x480809C)
#define REG_WIFI_TXQ0_ADR               (*(vu16*)0x48080A0)
#define REG_WIFI_TXQ1_ADR               (*(vu16*)0x48080A4)
#define REG_WIFI_TXQ2_ADR               (*(vu16*)0x48080A8)
#define REG_WIFI_TXQ_CLOSE              (*(vu16*)0x48080AC)
#define REG_WIFI_TXQ_OPEN               (*(vu16*)0x48080AE)
#define REG_WIFI_TXQ_MAP                (*(vu16*)0x48080B0)
#define REG_WIFI_TXQ_RESET              (*(vu16*)0x48080B4)
#define REG_WIFI_TXQ_CURR               (*(vu16*)0x48080B6)
#define REG_WIFI_TX_STATUS              (*(vu16*)0x48080B8)
#define REG_WIFI_TX_ERR_STATUS          (*(vu16*)0x48080BA)
#define REG_WIFI_TX_PREAMBLE_TYPE	    (*(vu16*)0x48080BC)
#define REG_WIFI_INITIAL_MP_DURATION    (*(vu16*)0x48080C0)
#define REG_WIFI_KEYSHARING_TXOP        (*(vu16*)0x48080C4)
#define REG_WIFI_KEYSHARING_STATUS      (*(vu16*)0x48080C8)

#define REG_WIFI_BUFFERING_SELECT       (*(vu16*)0x48080D0)
#define REG_WIFI_RESPONSE_CONTROL       (*(vu16*)0x48080D4)
#define REG_WIFI_OVF_THRESHOLD          (*(vu16*)0x48080D8)
#define REG_WIFI_DEFRAG_OFFSET          (*(vu16*)0x48080DA)
#define REG_WIFI_DS_MASK                (*(vu16*)0x48080E0)
#define REG_WIFI_TSF_ENABLE             (*(vu16*)0x48080E8)
#define REG_WIFI_TBTT_ENABLE            (*(vu16*)0x48080EA)
#define REG_WIFI_NAV_ENABLE             (*(vu16*)0x48080EC)
#define REG_WIFI_TMPTT_ENABLE           (*(vu16*)0x48080EE)
#define REG_WIFI_NEXT_TBTT_TSF_0        (*(vu16*)0x48080F0)
#define REG_WIFI_NEXT_TBTT_TSF_1        (*(vu16*)0x48080F2)
#define REG_WIFI_NEXT_TBTT_TSF_2        (*(vu16*)0x48080F4)
#define REG_WIFI_NEXT_TBTT_TSF_3        (*(vu16*)0x48080F6)
#define REG_WIFI_TSF_0                  (*(vu64*)0x48080F8)
#define REG_WIFI_TSF_1                  (*(vu16*)0x48080FA)
#define REG_WIFI_TSF_2                  (*(vu16*)0x48080FC)
#define REG_WIFI_TSF_3                  (*(vu16*)0x48080FE)
#define REG_WIFI_RXED_TSF_TIME_0        (*(vu16*)0x4808100)
#define REG_WIFI_RXED_TSF_TIME_1        (*(vu16*)0x4808102)
#define REG_WIFI_RXED_TSF_TIME_2        (*(vu16*)0x4808104)
#define REG_WIFI_RXED_TSF_TIME_3        (*(vu16*)0x4808106)
#define REG_WIFI_NAV                    (*(vu16*)0x480810C)
#define REG_WIFI_PRE_TBTT               (*(vu16*)0x4808110)
#define REG_WIFI_TMPTT_TIMER            (*(vu16*)0x4808118)
#define REG_WIFI_TBTT_TIMER             (*(vu16*)0x480811C)
#define REG_WIFI_RDY_TIMEOUT            (*(vu16*)0x4808120)
#define REG_WIFI_RX_TIMEOUT             (*(vu16*)0x4808122)
#define REG_WIFI_TBTT_ACT_TIME          (*(vu16*)0x4808124)
#define REG_WIFI_TBTT_WAKEUP_TIME       (*(vu16*)0x4808126)
#define REG_WIFI_TMPTT_ACT_TIME         (*(vu16*)0x4808128)
#define REG_WIFI_TMPTT_WAKEUP_TIME      (*(vu16*)0x480812A)
#define REG_WIFI_TIMEOUT_PARAM          (*(vu16*)0x4808130)
#define REG_WIFI_ACK_CCA_TIMEOUT        (*(vu16*)0x4808132)
#define REG_WIFI_ACTIVE_ZONE_TIMER      (*(vu16*)0x4808134)
#define REG_WIFI_TSF_TXOFFSET           (*(vu16*)0x4808140)
#define REG_WIFI_TSF_RXOFFSET           (*(vu16*)0x4808142)
#define REG_WIFI_CCA_DELAY              (*(vu16*)0x4808144)
#define REG_WIFI_TXPE_OFF_DELAY         (*(vu16*)0x4808146)
#define REG_WIFI_TX_DELAY               (*(vu16*)0x4808148)
#define REG_WIFI_RX_DELAY               (*(vu16*)0x480814A)
#define REG_WIFI_TRX_PE_INTERVAL        (*(vu16*)0x480814C)
#define REG_WIFI_RF_WAKEUP_TIME         (*(vu16*)0x4808150)
#define REG_WIFI_MULTIACK_DELAY_TIME    (*(vu16*)0x4808154)

#define REG_WIFI_BCR_CMDADR             (*(vu16*)0x4808158)
#define REG_WIFI_BCR_WDAT               (*(vu16*)0x480815A)
#define REG_WIFI_BCR_RDAT               (*(vu16*)0x480815C)
#define REG_WIFI_BCR_STATUS             (*(vu16*)0x480815E)
#define REG_WIFI_BCR_CONFIG             (*(vu16*)0x4808160)
#define REG_WIFI_RFR_D_CTRL             (*(vu16*)0x4808168)
#define REG_WIFI_RFR_D_STATUS           (*(vu16*)0x480816A)
#define REG_WIFI_RFR_CMD                (*(vu16*)0x480817C)
#define REG_WIFI_RFR_DAT                (*(vu16*)0x480817E)
#define REG_WIFI_RFR_STATUS             (*(vu16*)0x4808180)
#define REG_WIFI_RFR_CONFIG             (*(vu16*)0x4808184)

#define REG_WIFI_MP_BURST_TEST          (*(vu16*)0x4808190)
#define REG_WIFI_TX_TEST_FRAME          (*(vu16*)0x4808194)
#define REG_WIFI_RX_TEST_MODE           (*(vu16*)0x4808198)
#define REG_WIFI_SIGNAL_STATE           (*(vu16*)0x480819C)
#define REG_WIFI_TRPE_DIRECT_CTL        (*(vu16*)0x48081A0)
#define REG_WIFI_SERIAL_DAT_SELECT      (*(vu16*)0x48081A2)
#define REG_WIFI_SERIAL_DAT_DIRECT      (*(vu16*)0x48081A4)
#define REG_WIFI_RX_CNTUP_STATUS        (*(vu16*)0x48081A8)
#define REG_WIFI_RX_CNTUP_INT_MASK      (*(vu16*)0x48081AA)
#define REG_WIFI_RX_CNTOVF_STATUS       (*(vu16*)0x48081AC)
#define REG_WIFI_RX_CNTOVF_INT_MASK     (*(vu16*)0x48081AE)
#define REG_WIFI_PLCP_CRC_ERR           (*(vu16*)0x48081B0)
#define REG_WIFI_PLCP_FIELD_ERR         (*(vu16*)0x48081B2)
#define REG_WIFI_BUF_ERR                (*(vu16*)0x48081B4)
#define REG_WIFI_FCS_ERR                (*(vu16*)0x48081B6)
#define REG_WIFI_FC_ERR                 (*(vu16*)0x48081B8)
#define REG_WIFI_RX_RTS                 (*(vu16*)0x48081BA)
#define REG_WIFI_DUP_ERR                (*(vu16*)0x48081BE)
#define REG_WIFI_ACK_ERR_CNT            (*(vu16*)0x48081C0)
#define REG_WIFI_RXS_CNT                (*(vu16*)0x48081C4)
#define REG_WIFI_RESP_ERR_CNT01         (*(vu16*)0x48081D0)
#define REG_WIFI_RESP_ERR_CNT23         (*(vu16*)0x48081D2)
#define REG_WIFI_RESP_ERR_CNT45         (*(vu16*)0x48081D4)
#define REG_WIFI_RESP_ERR_CNT67         (*(vu16*)0x48081D6)
#define REG_WIFI_RESP_ERR_CNT89         (*(vu16*)0x48081D8)
#define REG_WIFI_RESP_ERR_CNTAB         (*(vu16*)0x48081DA)
#define REG_WIFI_RESP_ERR_CNTCD         (*(vu16*)0x48081DC)
#define REG_WIFI_RESP_ERR_CNTEF         (*(vu16*)0x48081DE)
#define REG_WIFI_M_TST                  (*(vu16*)0x48081F0)

#define REG_WIFI_WEP_STATUS             (*(vu16*)0x4808204)
#define REG_WIFI_CURR_SEQCTRL           (*(vu16*)0x4808210)
#define REG_WIFI_MAC_STATE              (*(vu16*)0x4808214)
#define REG_WIFI_8228                   (*(vu16*)0x4808228)
#define REG_WIFI_TXS_TX_STATUS          (*(vu16*)0x480823C)
#define REG_WIFI_MDP_JUMP_TEST          (*(vu16*)0x4808244)
#define REG_WIFI_WDP_LAST_RX_ADR        (*(volatile wifi_macaddr_t*)0x480824C)
#define REG_WIFI_DDO_PARA_DAT           (*(vu16*)0x4808254)
#define REG_WIFI_WDP_STATUS             (*(vu16*)0x480825C)
#define REG_WIFI_WDP_BYTE_COUNT         (*(vu16*)0x4808260)
#define REG_WIFI_WDP_BUF_START_ADR      (*(vu16*)0x4808264)
#define REG_WIFI_WDP_CUR_BUF_ADR        (*(vu16*)0x4808268)
#define REG_WIFI_WDP_MSDU_END_ADR       (*(vu16*)0x4808270)
#define REG_WIFI_WDP_LAST_RX_SEQ_CTR    (*(vu16*)0x4808278)
#define REG_WIFI_8290                   (*(vu16*)0x4808290)
#define REG_WIFI_82B8                   (*(vu16*)0x48082B8)
#define REG_WIFI_WAKEUP_TIMER           (*(vu16*)0x48082D0)

// == REG_WIFI_VERSION ==
#define WIFI_VERSION_NTR            0x1440
#define WIFI_VERSION_USG            0xC340

// == REG_WIFI_ISR and REG_WIFI_IMR ==

#define WIFI_IRQ_RX_END             (1 << 0)
#define WIFI_IRQ_TX_END             (1 << 1)
#define WIFI_IRQ_RX_COUNT           (1 << 2)
#define WIFI_IRQ_TX_ERROR           (1 << 3)
#define WIFI_IRQ_RX_COUNT_OVERFLOW  (1 << 4)
#define WIFI_IRQ_TX_ERROR_OVERFLOW  (1 << 5)
#define WIFI_IRQ_RX_START           (1 << 6)
#define WIFI_IRQ_TX_START           (1 << 7)
#define WIFI_IRQ_TXBUF_COUNT_EXP    (1 << 8)
#define WIFI_IRQ_RXBUF_COUNT_EXP    (1 << 9)
#define WIFI_IRQ_RF_WAKEUP          (1 << 11)
#define WIFI_IRQ_MP_END             (1 << 12)   // multipoll sequence end
#define WIFI_IRQ_ACT_END            (1 << 13)   // active zone end
#define WIFI_IRQ_TBTT               (1 << 14)   // Target Beacon Transmission Time (TBTT)
#define WIFI_IRQ_PRE_TBTT           (1 << 15)

// == REG_WIFI_MDP_CONFIG ==

#define WIFI_MDP_CONFIG_SET_RX_BUF_EMPTY    (1 << 0)
#define WIFI_MDP_CONFIG_MP_KEYIN_TO_KEYOUT  (1 << 7)
#define WIFI_MDP_CONFIG_RX_BUF_ENABLE       (1 << 15)

// == REG_WIFI_WAKEUP_CTRL ==

#define WIFI_WAKEUP_CTRL_WAKEUP_TBTT        (1 << 0)
#define WIFI_WAKEUP_CTRL_TMPTT_POWER_SAVE   (1 << 1)

// == REG_WIFI_SET_POWER ==

#define WIFI_POWER_STATE_SLEEP      0
#define WIFI_POWER_STATE_1          1
#define WIFI_POWER_STATE_ACTIVE     2

// == REG_WIFI_TX_STATUS ==

#define WIFI_TX_STATUS_FRAME_TYPE_SHIFT     8

#define WIFI_TX_STATUS_FRAME_TYPE_BEACON    (3 << WIFI_TX_STATUS_FRAME_TYPE_SHIFT)
#define WIFI_TX_STATUS_FRAME_TYPE_MP_REPLY  (4 << WIFI_TX_STATUS_FRAME_TYPE_SHIFT)
#define WIFI_TX_STATUS_FRAME_TYPE_MP        (8 << WIFI_TX_STATUS_FRAME_TYPE_SHIFT)
#define WIFI_TX_STATUS_FRAME_TYPE_MP_ACK    (11 << WIFI_TX_STATUS_FRAME_TYPE_SHIFT)

#define WIFI_TX_STATUS_FRAME_TYPE_MASK      (0xF << WIFI_TX_STATUS_FRAME_TYPE_SHIFT)

// == REG_WIFI_DS_MASK ==

#define WIFI_DS_MASK_IGNORE_STA_TO_STA      (1 << 0)
#define WIFI_DS_MASK_IGNORE_STA_TO_DS       (1 << 1)
#define WIFI_DS_MASK_IGNORE_DS_TO_STA       (1 << 2)
#define WIFI_DS_MASK_IGNORE_DS_TO_DS        (1 << 3)

// == REG_WIFI_MAC_STATE ==

#define WIFI_MAC_STATE_0            0
#define WIFI_MAC_STATE_2            2
#define WIFI_MAC_STATE_TX           3
#define WIFI_MAC_STATE_WAIT         5
#define WIFI_MAC_STATE_7            7
#define WIFI_MAC_STATE_8            8

// ==  ==

#define WIFI_TXQ_ENABLE             (1 << 15)

#define WIFI_TXQ_TXQ0               (1 << 0)    // data
#define WIFI_TXQ_MP                 (1 << 1)
#define WIFI_TXQ_TXQ1               (1 << 2)    // manctrl
#define WIFI_TXQ_TXQ2               (1 << 3)    // broadcast
#define WIFI_TXQ_BEACON             (1 << 4)

#define WIFI_TXQ_TXQ_ALL            (WIFI_TXQ_TXQ0 | WIFI_TXQ_TXQ1 | WIFI_TXQ_TXQ2)

// == REG_WIFI_TXQ_RESET ==

#define WIFI_TXQ_RESET_TXQ0         (1 << 0)
#define WIFI_TXQ_RESET_MP           (1 << 1)
#define WIFI_TXQ_RESET_TXQ1         (1 << 2)
#define WIFI_TXQ_RESET_TXQ2         (1 << 3)
#define WIFI_TXQ_RESET_KEYOUT       (1 << 6)
#define WIFI_TXQ_RESET_KEYIN        (1 << 7)

// == REG_WIFI_SIGNAL_STATE ==

#define WIFI_SIGNAL_STATE_CCA       (1 << 0)
#define WIFI_SIGNAL_STATE_TRRDY     (1 << 1)
#define WIFI_SIGNAL_STATE_TXPE      (1 << 6)
#define WIFI_SIGNAL_STATE_RXPE      (1 << 7)

// == REG_WIFI_MDP_JUMP_TEST ==

#define WIFI_MDP_JUMP_TEST_MP_RECEIVED_CLEAR_SEQUENCE   (1 << 6)