#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern u32 gIrqYieldingEnabled;

extern void vm_enableNestedIrqs(void);
extern void vm_disableNestedIrqs(void);
extern bool vm_yieldGbaIrqs(void);
extern bool vm_disableIrqYielding(void);
extern void vm_restoreIrqYielding(bool isIrqYieldingEnabled);

#ifdef __cplusplus
}
#endif
