#include "common.h"
#include "SdCache/SdCache.h"
#include "cp15.h"
#include "JitCommon.h"
#include "JitThumb.h"

// #define TRACE_THUMB_UNDEFINED

static void __attribute__((noinline)) thumbJitNotImplemented()
{
    asm volatile ("bkpt #0");
    while (1);
}

void jit_processThumbBlock(u16* ptr)
{
    void* const blockStart = jit_findBlockStart(ptr);
    void* blockEnd = jit_findBlockEnd(ptr);
    u32* jitBits = jit_getJitBits(ptr);
    do
    {
        if (jitBits)
        {
            u32 bitIdx = ((u32)ptr & 0x3F) >> 1;
            u32 bitMask = 1 << bitIdx;
            if (*jitBits & bitMask)
            {
                // stop because this instruction was already processed
                break;
            }
            *jitBits |= bitMask;
            if (bitIdx == 31)
                jitBits++;
        }
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
            *(u16*)(((u32)ptr & ~0x01000000) + 0x00400000) = instruction;
            *ptr = 0xB100;
            break;
        }
        else if ((instruction & 0xF800) == 0xE000)
        {
            // b
            *(u16*)(((u32)ptr & ~0x01000000) + 0x00400000) = instruction;
            *ptr = 0xB100;
            break;
        }
        else if ((instruction & 0xF800) == 0xF800)
        {
            // bl lr+imm
            *(u16*)(((u32)ptr & ~0x01000000) + 0x00400000) = instruction;
            *ptr = 0xB100;
            break;
        }
        else if ((instruction & 0xFF87) == 0x4700)
        {
            // bx Rm
            *ptr = 0xDE00 | ((instruction >> 3) & 0xF);
            break;
        }
        else if ((instruction & 0xFF87) == 0x4487)
        {
            // add pc, Rm
            *(u16*)(((u32)ptr & ~0x01000000) + 0x00400000) = instruction;
            *ptr = 0xB100;
            break;
        }
        else if ((instruction & 0xFF87) == 0x4687)
        {
            // mov pc, Rm
            // if (((u32)ptr >> 24) == 2)
            // {
            //     *(u16*)((u32)ptr + 0x0400000) = instruction;
            // }
            *(u16*)(((u32)ptr & ~0x01000000) + 0x00400000) = instruction;
            *ptr = 0xB100;
            break;
        }
        else if ((instruction & 0xFF00) == 0xBD00)
        {
            // pop pc
            // if (((u32)ptr >> 24) == 2)
            // {
            //     *(u16*)((u32)ptr + 0x0400000) = instruction;
            // }
            *(u16*)(((u32)ptr & ~0x01000000) + 0x00400000) = instruction;
            *ptr = 0xB100;
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
                continue;
            }
        }
    } while ((u32)++ptr < (u32)blockEnd);
}

[[gnu::section(".itcm")]]
u16* jit_handleThumbUndefined(u32 instruction, u16* instructionPtr, u32* registers, u32 cpsr)
{
    if ((instruction & 0xF000) == 0xD000)
    {
        // b cond
        u32 condition = (instruction >> 8) & 0xF;
        bool conditionPass = jit_conditionPass(cpsr, condition);
        u32 branchDestination = ((u32)instructionPtr + 4 + ((int)(instruction << 24) >> 23)) | 1;
        bool putBack;
        u32 address;
        if (conditionPass)
        {
            address = branchDestination;
            jit_ensureBlockJitted((void*)address);
            putBack = jit_isBlockJitted((void*)((u32)instructionPtr + 3));
        }
        else
        {
            address = (u32)instructionPtr + 3;
            jit_ensureBlockJitted((void*)address);
            putBack = jit_isBlockJitted((void*)branchDestination);
        }
        if (putBack)
        {
            *instructionPtr = instruction;
            dc_drainWriteBuffer();
            ic_invalidateAll();
        }
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xD000);
        logAddress(address);
#endif
        return (u16*)address;
    }
    else if ((instruction & 0xF800) == 0xE000)
    {
        // b
        u32 branchDestination = ((u32)instructionPtr + 4 + ((int)(instruction << 21) >> 20)) | 1;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xE000);
        logAddress(branchDestination);
#endif
        jit_ensureBlockJitted((void*)branchDestination);
        *instructionPtr = instruction;
        dc_drainWriteBuffer();
        ic_invalidateAll();
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xF800) == 0xF800)
    {
        // bl lr+imm
        // todo: check if previous instruction was actually the first bl part
        u32 branchDestination = (registers[14] + ((instruction << 21) >> 20)) | 1;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xF800);
        logAddress(branchDestination);
#endif
        jit_ensureBlockJitted((void*)branchDestination);
        *instructionPtr = instruction;
        dc_drainWriteBuffer();
        ic_invalidateAll();
        registers[14] = (u32)instructionPtr + 3;
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xFF87) == 0x4487)
    {
        // add pc, Rm
        u32 branchDestination = ((u32)instructionPtr + 4 + registers[(instruction >> 3) & 0xF]) | 1;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0x4487);
        logAddress(branchDestination);
#endif
        jit_ensureBlockJitted((void*)branchDestination);
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xFF87) == 0x4687)
    {
        // mov pc, Rm
        u32 branchDestination = registers[(instruction >> 3) & 0xF] | 1;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0x4687);
        logAddress(branchDestination);
#endif
        jit_ensureBlockJitted((void*)branchDestination);
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xFF00) == 0xBD00)
    {
        // pop pc
        u32* sp = (u32*)registers[13];
        for (u32 i = 0; i < 8; i++)
        {
            if (instruction & (1 << i))
            {
                registers[i] = *sp++;
            }
        }
        u32 branchDestination = (*sp++) | 1;
        registers[13] = (u32)sp;
#ifdef TRACE_THUMB_UNDEFINED
        logAddress(0xBD00);
        logAddress(branchDestination);
#endif
        jit_ensureBlockJitted((void*)branchDestination);
        return (u16*)branchDestination;
    }
    else if ((instruction & 0xF800) == 0x4800)
    {
        // ldr Rd, [pc, #imm]
        thumbJitNotImplemented();
    }
    thumbJitNotImplemented();
    return (u16*)((u32)instructionPtr + 3);
}
