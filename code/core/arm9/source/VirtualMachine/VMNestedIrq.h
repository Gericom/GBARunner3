#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void vm_enableNestedIrqs(void);
extern void vm_disableNestedIrqs(void);

#ifdef __cplusplus
}
#endif
