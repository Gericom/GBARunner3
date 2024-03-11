#include "common.h"

#pragma GCC optimize ("Oz")

#define RN_TABLE        0
#define RD_TABLE        1
#define RM_TABLE        2

#define INDEX_L_BIT     (1 << 2)
#define INDEX_W_BIT     (1 << 3)
#define INDEX_U_BIT     (1 << 5)
#define INDEX_P_BIT     (1 << 6)

extern const void* memu_armLoadStoreShortRmTable_0[16];
extern const void* memu_armLoadStoreShortRmTable_1[16];
extern const void* memu_armLoadStoreShortImmTable_0[16];
extern const void* memu_armLoadStoreShortImmTable_1[16];
extern const void* memu_armLoadStoreRmTable_0[16];
extern const void* memu_armLoadStoreRmTable_1[16];
extern const void* memu_armLoadStoreImmTable_0[16];
extern const void* memu_armLoadStoreImmTable_1[16];
extern const void* memu_armLdmStmRmTable_00[16];
extern const void* memu_armLdmStmRmTable_01[16];
extern const void* memu_armLdmStmRmTable_10[16];
extern const void* memu_armLdmStmRmTable_11[16];
extern const void* memu_armLoadStoreRnTable_00[16];
extern const void* memu_armLoadStoreRnTable_01[16];
extern const void* memu_armLoadStoreRnTable_10[16];
extern const void* memu_armLoadStoreRnTable_11[16];
extern const void* memu_armSwpRnTable[16];
extern const void* memu_armLdmStmRnTable_0[16];
extern const void* memu_armLdmStmRnTable_1[16];
extern const void* memu_armSwpbRdTable[16];
extern const void* memu_armSwpRdTable[16];
extern const void* memu_armLdrhRdTable[16];
extern const void* memu_armStrhRdTable[16];
extern const void* memu_armLdrsbRdTable[16];
extern const void* memu_armLdrshRdTable[16];
extern const void* memu_armLdrRdTable[16];
extern const void* memu_armStrRdTable[16];
extern const void* memu_armLdrbRdTable[16];
extern const void* memu_armStrbRdTable[16];
extern const void* memu_armLdmRdTable[16];
extern const void* memu_armStmRdTable[16];

DTCM_DATA u8 memu_armDispatchTable[3][0x280];

static bool isLoad(u32 index)
{
    return index & INDEX_L_BIT;
}

static bool isWriteback(u32 index)
{
    return index & INDEX_W_BIT;
}

static void setTableEntry(u32 table, u32 index, const void* value)
{
    memu_armDispatchTable[table][index] = -(int)value >> 6;
}

static void setSwpSwpbTableEntries(u32 index)
{
    setTableEntry(RM_TABLE, index, memu_armLoadStoreShortRmTable_1);
    setTableEntry(RN_TABLE, index, memu_armSwpRnTable);
    if (index & (1 << 4)) // B
    {
        setTableEntry(RD_TABLE, index, memu_armSwpbRdTable);
    }
    else
    {
        setTableEntry(RD_TABLE, index, memu_armSwpRdTable);
    }
}

static void setLoadStoreShortRmEntry(u32 index)
{
    if (index & (1 << 4)) // I
    {
        setTableEntry(RM_TABLE, index,
            (index & INDEX_U_BIT) ? memu_armLoadStoreShortImmTable_1 : memu_armLoadStoreShortImmTable_0);
    }
    else
    {
        setTableEntry(RM_TABLE, index,
            (index & INDEX_U_BIT) ? memu_armLoadStoreShortRmTable_1 : memu_armLoadStoreShortRmTable_0);
    }
}

static void setLoadStoreRnEntry(u32 index)
{
    if (index & INDEX_P_BIT)
    {
        setTableEntry(RN_TABLE, index,
            isWriteback(index) ? memu_armLoadStoreRnTable_11 : memu_armLoadStoreRnTable_10);
    }
    else
    {
        setTableEntry(RN_TABLE, index,
            isWriteback(index) ? memu_armLoadStoreRnTable_01 : memu_armLoadStoreRnTable_00);
    }
}

static void setLdrhStrhTableEntries(u32 index)
{
    setLoadStoreShortRmEntry(index);
    setLoadStoreRnEntry(index);
    setTableEntry(RD_TABLE, index, isLoad(index) ? memu_armLdrhRdTable : memu_armStrhRdTable);
}

static void setLdrsbTableEntries(u32 index)
{
    setLoadStoreShortRmEntry(index);
    setLoadStoreRnEntry(index);
    setTableEntry(RD_TABLE, index, memu_armLdrsbRdTable);
}

static void setLdrshTableEntries(u32 index)
{
    setLoadStoreShortRmEntry(index);
    setLoadStoreRnEntry(index);
    setTableEntry(RD_TABLE, index, memu_armLdrshRdTable);
}

static void setLdrLdrbStrStrbTableEntries(u32 index)
{
    if (index & (1 << 7)) // I
    {
        setTableEntry(RM_TABLE, index,
            (index & INDEX_U_BIT) ? memu_armLoadStoreRmTable_1 : memu_armLoadStoreRmTable_0);
    }
    else
    {
        setTableEntry(RM_TABLE, index,
            (index & INDEX_U_BIT) ? memu_armLoadStoreImmTable_1 : memu_armLoadStoreImmTable_0);
    }
    setLoadStoreRnEntry(index);
    if (index & (1 << 4)) // B
    {
        setTableEntry(RD_TABLE, index, isLoad(index) ? memu_armLdrbRdTable : memu_armStrbRdTable);
    }
    else
    {
        setTableEntry(RD_TABLE, index, isLoad(index) ? memu_armLdrRdTable : memu_armStrRdTable);
    }
}

static void setLdmStmTableEntries(u32 index)
{
    if (index & INDEX_P_BIT)
    {
        setTableEntry(RM_TABLE, index,
            (index & INDEX_U_BIT) ? memu_armLdmStmRmTable_11 : memu_armLdmStmRmTable_10);
    }
    else
    {
        setTableEntry(RM_TABLE, index,
            (index & INDEX_U_BIT) ? memu_armLdmStmRmTable_01 : memu_armLdmStmRmTable_00);
    }
    setTableEntry(RN_TABLE, index,
        isWriteback(index) ? memu_armLdmStmRnTable_1 : memu_armLdmStmRnTable_0);
    setTableEntry(RD_TABLE, index,
        isLoad(index) ? memu_armLdmRdTable : memu_armStmRdTable);
}

void memu_initializeArmDispatchTable()
{
    for (u32 index = 0; index < 0x280; ++index)
    {
        u32 type = index >> 8;
        switch (type)
        {
            case 0:
            {
                u32 subType = index & 3;
                switch (subType)
                {
                    case 0:
                    {
                        if ((index & INDEX_P_BIT) && !(index & INDEX_U_BIT) && !isWriteback(index) && !isLoad(index))
                        {
                            setSwpSwpbTableEntries(index);
                        }
                        break;
                    }
                    case 1:
                    {
                        setLdrhStrhTableEntries(index);
                        break;
                    }
                    case 2:
                    {
                        if (isLoad(index))
                        {
                            setLdrsbTableEntries(index);
                        }
                        break;
                    }
                    case 3:
                    {
                        if (isLoad(index))
                        {
                            setLdrshTableEntries(index);
                        }
                        break;
                    }
                }
                break;
            }
            case 1:
            {
                setLdrLdrbStrStrbTableEntries(index);
                break;
            }
            case 2:
            {
                setLdmStmTableEntries(index);
                break;
            }
        }
    }
}
