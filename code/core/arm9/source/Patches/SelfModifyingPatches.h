#pragma once
class RunSettings;

class SelfModifyingPatches
{
    u32 _patchCodeOffset = 0;

    void PatchSelfModifyingWrite(u32 gbaAddress);
    bool ModifyStr(u32 &instruction);
    bool ModifyStrh(u32 &instruction);

public:
    void ApplyPatches(const RunSettings &runSettings);
};
