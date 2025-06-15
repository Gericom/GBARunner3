#pragma once

enum class ScfgArm9Clock
{
    Nitro67MHz = 0,
    Twl134MHz = 1
};

extern "C" void scfg_setArm9Clock(ScfgArm9Clock arm9Clock);