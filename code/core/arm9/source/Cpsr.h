#pragma once

#ifdef __cplusplus
extern "C" {
#endif

static inline u32 arm_getCpsr()
{
    u32 cpsr;
    asm volatile("mrs %0, cpsr" : "=r" (cpsr));
    return cpsr;
}

static inline void arm_setCpsrControl(u32 cpsrControl)
{
    asm volatile("msr cpsr_c, %0" :: "r" (cpsrControl) : "cc");
}

static inline u32 arm_disableIrqs(void)
{
    u32 oldCpsr = arm_getCpsr();
    arm_setCpsrControl(oldCpsr | 0x80);
    return oldCpsr;
}

static inline u32 arm_enableIrqs(void)
{
    u32 oldCpsr = arm_getCpsr();
    arm_setCpsrControl(oldCpsr & ~0x80);
    return oldCpsr;
}

static inline void arm_restoreIrqs(u32 oldCpsr)
{
    arm_setCpsrControl(oldCpsr);
}

#ifdef __cplusplus
}
#endif
