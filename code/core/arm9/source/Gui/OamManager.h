#pragma once
#include <libtwl/gfx/gfxOam.h>

class alignas(32) OamManager
{
    gfx_oam_table_t _oamTable alignas(32);
    gfx_oam_entry_t* _oamPtr;
    u32 _mtxIdx;

  public:
    OamManager()
    {
        Clear();
    }

    gfx_oam_entry_t* AllocOams(u32 count)
    {
        _oamPtr -= count;
        return _oamPtr;
    }

    gfx_oam_mtx_t* AllocMatrices(u32 count, u32& mtxId)
    {
        mtxId = _mtxIdx;
        gfx_oam_mtx_t* result = &_oamTable.mtxs[_mtxIdx];
        _mtxIdx += count;
        return result;
    }

    void Apply(vu16* dst);

    void Clear();
};
