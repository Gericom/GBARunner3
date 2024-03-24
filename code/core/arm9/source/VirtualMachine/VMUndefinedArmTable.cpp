#include "common.h"
#include "VMUndefinedArmTable.h"

#pragma GCC optimize ("Oz")

extern const void* vm_armUndefinedMsrRegSpsrRmTable[16];
extern const void* vm_armUndefinedMsrRegCpsrRmTable[16];
extern const void* vm_armUndefinedMrsSpsrRmTable[16];
extern const void* vm_armUndefinedMrsCpsrRmTable[16];
extern const void* jit_armUndefinedBxRmTable[16];
extern const void* jit_armUndefinedBTable[16];
extern const void* jit_armUndefinedBLTable[16];
extern const void* jit_armUndefinedLdrPcImmRnTable[16];
extern const void* vm_armUndefinedAluSPCImmRnTable[16];

DTCM_DATA u8 vm_undefinedArmTable[1024];

static void setTableEntry(u32 index, const void* value)
{
    vm_undefinedArmTable[index] = -(int)value >> 6;
}

void vm_initializeUndefinedArmTable()
{
    for (u32 index = 0; index < 1024; ++index)
    {
        u32 prefix = (index >> 5) & 7;
        u32 d = (index >> 4) & 1;
        u32 e = (index >> 3) & 1;
        u32 f = (index >> 2) & 1;
        u32 g = (index >> 1) & 1;
        u32 h = (index >> 0) & 1;
        u32 x = (index >> 8) & 1;
        u32 y = (index >> 9) & 1;

        if ((prefix == 0b000) && (d == 1) && (e == 1) && (g == 0))
        {
            // MSR
            if (h == 1)
            {
                // setTableEntry(index, vm_armUndefinedMsrImm\f)
                setTableEntry(index, nullptr);
            }
            else
            {
                setTableEntry(index, f ? vm_armUndefinedMsrRegSpsrRmTable : vm_armUndefinedMsrRegCpsrRmTable);
            }
        }
        else if ((prefix == 0b000) && (d == 1) && (e == 1) && (g == 1) && (h == 0))
        {
            // MRS
            setTableEntry(index, f ? vm_armUndefinedMrsSpsrRmTable : vm_armUndefinedMrsCpsrRmTable);
        }
    #ifndef GBAR3_TEST
        else if ((prefix == 0b000) && (d == 1) && (e == 1) && (f == 0) && (g == 1) && (h == 1))
        {
            // BX
            setTableEntry(index, jit_armUndefinedBxRmTable);
        }
        else if ((prefix == 0b110) && (d == 0))
        {
            // B
            setTableEntry(index, jit_armUndefinedBTable);
        }
        else if ((prefix == 0b110) && (d == 1))
        {
            // BL
            setTableEntry(index, jit_armUndefinedBLTable);
        }
        else if ((prefix == 0b111) && (d == 0) && (e == 1) && (f == 0) && (g == 0) && (h == 0) && (x == 0) && (y == 1))
        {
            // LDR pc, [Rn, +/-#imm] or LDR pc, [Rn], +/-#imm
            setTableEntry(index, jit_armUndefinedLdrPcImmRnTable);
        }
    #endif
        else if ((prefix == 0b111) && (d == 0) && (e == 0) && (y == 0))
        {
            // ALU pc
            if ((f == 1) && (g == 1))
            {
                // ALUs pc, rn, #imm
                setTableEntry(index, vm_armUndefinedAluSPCImmRnTable);
            }
            else
            {
                setTableEntry(index, nullptr);
            }
        }
        else
        {
            setTableEntry(index, nullptr);
        }
    }
}
