#include "common.h"
#include "SdCache/SdCache.h"
#include "JitCommon.h"

void jit_processArmBlock(u32* ptr)
{
    u32* const blockStart = jit_findBlockStart(ptr);
    u32* const blockEnd = jit_findBlockEnd(ptr);
    u32* jitBits = jit_getJitBits(ptr);
    do
    {
        if (jitBits)
        {
            u32 bitIdx = ((u32)ptr & 0x3F) >> 1;
            *jitBits |= 3 << bitIdx;
            if (bitIdx == 30)
                jitBits++;
        }
        u32 instruction = *ptr;
        if ((instruction & 0x0E000000) == 0x0A000000)
        {
            // B and BL imm
            *ptr = (instruction & ~0x0E000000) | 0x0C000000;
            break;
        }
        else if ((instruction & 0x0FBF0FFF) == 0x010F0000)
        {
            // MRS
            *ptr = 0x01A00090 | (instruction & 0xF0400000) | ((instruction & 0x0000F000) >> 12);
        }
        else if ((instruction & 0x0FB0FFF0) == 0x0120F000)
        {
            // MSR reg
            *ptr = 0x1800090 | (instruction & 0xF04F000F) | ((instruction & 0x02000000) >> 5);
        }
        else if ((instruction & 0x0FFFFFF0) == 0x012FFF10)
        {
            // BX
            *ptr = 0x01B00090 | (instruction & 0xF000000F);
            break;
        }
        else if ((instruction & 0x0E108000) == 0x08108000)
        {
            // LDM pc
            break;
        }
        else if ((instruction & 0x0C50F000) == 0x0410F000)
        {
            // LDR pc
            break;
        }
        else if ((instruction & 0x0C5F0000) == 0x041F0000)
        {
            // LDR Rd, [pc, ...]
            if (!(instruction & 0x02000000))
            {
                // LDR Rd, [pc, #imm]
                int offset = instruction & 0xFFF;
                if (!(instruction & 0x00800000))
                    offset = -offset;                
                u32 targetAddress = (u32)ptr + 8 + offset;
                if (targetAddress >= (u32)blockStart && targetAddress < (u32)blockEnd)
                {
                    // safe pool address needs no patching
                    continue;
                }
            }
        }
        else if ((instruction & 0x0E00F010) == 0x0000F000)
        {
            // ALU{S} pc, Rn, Rm (imm shift)
            break;
        }
        else if ((instruction & 0x0E00F000) == 0x0200F000)
        {
            // ALU{S} pc, Rn, #imm
            if (instruction & 0x00100000)
            {
                // ALUS pc, Rn, #imm
                *ptr = 0x0E600000
                    | ((instruction & 0x01E00000) >> 4)
                    | ((instruction & 0x000F0000) >> 16)
                    | ((instruction & 0x00000FFF) << 5);
            }
            break;
        }
        else if ((instruction & 0x0F000000) == 0x0F000000)
        {
            // SWI
            break;
        }
    } while (++ptr < blockEnd);
}