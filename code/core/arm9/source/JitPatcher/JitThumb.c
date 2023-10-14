#include "common.h"
#include "SdCache/SdCache.h"
#include "cp15.h"
#include "JitCommon.h"
#include "JitThumb.h"

// #define TRACE_THUMB_UNDEFINED

[[gnu::noreturn, gnu::noinline]]
static void thumbJitNotImplemented()
{
    asm volatile ("bkpt #0");
    while (1);
}

void jit_processThumbBlock(u16* ptr)
{
    void* const blockStart = jit_findBlockStart(ptr);
    void* blockEnd = jit_findBlockEnd(ptr);
    u8* jitBits = jit_getJitBits(ptr);
    u16* jitAuxBits = jit_getJitAuxBits(ptr);
    do
    {
        u32 bitIdx = ((u32)ptr & 0xF) >> 1;
        u32 bitMask = 1 << bitIdx;
        if (*jitBits & bitMask)
        {
            // stop because this instruction was already processed
            break;
        }
        *jitBits |= bitMask;
        if (bitIdx == 7)
            jitBits++;
        u32 auxBitIdx = (u32)ptr & 0xF;
        u32 instruction = *ptr;
        if ((instruction & 0xFF00) == 0xDF00)
        {
            // SWI
            u32 swiOp = instruction & 0xFF;
            if (swiOp == 0 || swiOp == 0x26 || swiOp >= 0x80)
                break;
        }
        else if ((instruction & 0xF000) == 0xD000)
        {
            // b cond
            *ptr = 0b1011100000000000
                | (((instruction >> 8) & 0xF) << 6) // cond
                | ((instruction >> 2) & 0x3F); // offs[7:2]
            *jitAuxBits |= (instruction & 3) << auxBitIdx;
            break;
        }
        else if ((instruction & 0xF800) == 0xE000)
        {
            // b
            *ptr = 0b1011001000000000
                | (instruction & 0x5FF); // off[10] and offs[8:0]
            *jitAuxBits |= ((instruction >> 9) & 1) << auxBitIdx;
            break;
        }
        else if ((instruction & 0xF800) == 0xF800)
        {
            // bl lr+imm
            *ptr = 0b1110100000000001
                | (instruction & 0x7FE); // offs[10:1]
            *jitAuxBits |= (instruction & 1) << auxBitIdx;
            break;
        }
        else if ((instruction & 0xFF87) == 0x4700)
        {
            // bx Rm
            *ptr = 0b1011000100000000
                | (((instruction >> 3) & 0xF) << 4); // Rm
            break;
        }
        else if ((instruction & 0xFF87) == 0x4487)
        {
            // add pc, Rm
            *ptr = 0b1011111100000000
                | (((instruction >> 3) & 0xF) << 4); // Rm
            break;
        }
        else if ((instruction & 0xFF87) == 0x4687)
        {
            // mov pc, Rm
            *ptr = 0b1101111000000000
                | (((instruction >> 3) & 0xF) << 4); // Rm
            break;
        }
        else if ((instruction & 0xFF00) == 0xBD00)
        {
            // pop pc
            *ptr = 0b1011101110000000
                | ((instruction >> 1) & 0x7F); // rlist[7:1]
            *jitAuxBits |= (instruction & 1) << auxBitIdx;
            break;
        }
        else if ((instruction & 0xF800) == 0x4800)
        {
            // ldr Rd, [pc, #imm]
            u32 targetAddress = (((u32)ptr + 4) & ~2) + ((instruction & 0xFF) << 2);
            if (targetAddress >= (u32)blockStart && targetAddress < (u32)blockEnd)
            {
                if (targetAddress > (u32)ptr)
                    blockEnd = (void*)targetAddress;
                // safe pool address needs no patching
            }
            else
            {

            }
        }
        
        if (auxBitIdx == 0xE)
            jitAuxBits++;
    } while ((u32)++ptr < (u32)blockEnd);
}

[[gnu::section(".itcm")]]
u16* jit_handleThumbBCond(u16* instructionPtr, u32 instruction, bool conditionPass)
{
    // todo: check if previous instruction was actually the first bl part
    u16* jitAuxBits = jit_getJitAuxBits(instructionPtr);
    u32 auxBits = ((*jitAuxBits) >> ((u32)instructionPtr & 0xF)) & 3;

    u32 offset = ((instruction & 0x3F) << 2) | auxBits;

    u32 branchDestination = (u32)instructionPtr + 5 + ((int)(offset << 24) >> 23);

    u32 address;
    u32 otherAddress;
    if (conditionPass)
    {
        address = branchDestination;
        otherAddress = (u32)instructionPtr + 3;
    }
    else
    {
        address = (u32)instructionPtr + 3;
        otherAddress = branchDestination;
    }

    if (jit_isBlockJitted((void*)otherAddress))
    {
        u32 condition = (instruction >> 6) & 0xF;
        *instructionPtr = 0xD000 | (condition << 8) | offset;
        dc_drainWriteBuffer();
        ic_invalidateAll();
    }
#ifdef TRACE_THUMB_UNDEFINED
    logAddress(0xD000);
    logAddress(address);
#endif
    return (u16*)address;
}

[[gnu::section(".itcm")]]
u16* jit_handleThumbUndefined(u32 instruction, u16* instructionPtr, u32* registers)
{
    u16* jitAuxBits = jit_getJitAuxBits(instructionPtr);
    u32 auxBits = ((*jitAuxBits) >> ((u32)instructionPtr & 0xF)) & 3;
    if ((instruction & 0xFF80) == 0b1011101110000000)
    {
        // pop pc
        u32 rList = ((instruction & 0x7F) << 1) | (auxBits & 1);
        u32* sp = (u32*)registers[8];
        for (u32 i = 0; i < 8; ++i)
        {
            if (rList & (1 << i))
            {
                registers[i] = *sp++;
            }
        }
        u32 branchDestination = (*sp++) | 1;
        registers[8] = (u32)sp;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xBD00);
        logAddress(branchDestination);
#endif
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xFA00) == 0b1011001000000000)
    {
        // b
        u32 offset = (instruction & 0x5FF) | ((auxBits & 1) << 9);
        u32 branchDestination = ((u32)instructionPtr + 4 + ((int)(offset << 21) >> 20)) | 1;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xE000);
        logAddress(branchDestination);
#endif
        *instructionPtr = 0xE000 | offset;
        dc_drainWriteBuffer();
        ic_invalidateAll();
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xF801) == 0xE801)
    {
        // bl lr+imm
        // todo: check if previous instruction was actually the first bl part
        u32 offset = (instruction & 0x7FE) | (auxBits & 1);
        u32 branchDestination = (registers[9] + ((offset << 21) >> 20)) | 1;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xF800);
        logAddress(branchDestination);
#endif
        registers[9] = (u32)instructionPtr + 3;
        *instructionPtr = 0xF800 | offset;
        dc_drainWriteBuffer();
        ic_invalidateAll();
        return (u16*)branchDestination;
    }
    else
    {
        thumbJitNotImplemented();
        return (u16*)((u32)instructionPtr + 3);
    }
}
