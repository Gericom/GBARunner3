#pragma once

#define ARM_PATCH_SWI(x)    (0xEF000000 | ((0xA0 + (x)) << 16))

void patch_resetSwiPatches();
int patch_addSwiPatch(void* function);
