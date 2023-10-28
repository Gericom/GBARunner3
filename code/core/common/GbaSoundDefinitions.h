#pragma once

// REG_SOUNDCNT_H
#define GBA_SOUNDCNT_H_GB_VOLUME_QUARTER        0
#define GBA_SOUNDCNT_H_GB_VOLUME_HALF           1
#define GBA_SOUNDCNT_H_GB_VOLUME_FULL           2
#define GBA_SOUNDCNT_H_GB_VOLUME_MASK           3

#define GBA_SOUNDCNT_H_DIRECT_A_VOLUME_HALF     (0 << 2)
#define GBA_SOUNDCNT_H_DIRECT_A_VOLUME_FULL     (1 << 2)

#define GBA_SOUNDCNT_H_DIRECT_B_VOLUME_HALF     (0 << 3)
#define GBA_SOUNDCNT_H_DIRECT_B_VOLUME_FULL     (1 << 3)

#define GBA_SOUNDCNT_H_DIRECT_A_ENABLE_RIGHT    (1 << 8)
#define GBA_SOUNDCNT_H_DIRECT_A_ENABLE_LEFT     (1 << 9)

#define GBA_SOUNDCNT_H_DIRECT_A_TIMER_0         (0 << 10)
#define GBA_SOUNDCNT_H_DIRECT_A_TIMER_1         (1 << 10)

#define GBA_SOUNDCNT_H_DIRECT_A_RESET           (1 << 11)

#define GBA_SOUNDCNT_H_DIRECT_B_ENABLE_RIGHT    (1 << 12)
#define GBA_SOUNDCNT_H_DIRECT_B_ENABLE_LEFT     (1 << 13)

#define GBA_SOUNDCNT_H_DIRECT_B_TIMER_0         (0 << 14)
#define GBA_SOUNDCNT_H_DIRECT_B_TIMER_1         (1 << 14)

#define GBA_SOUNDCNT_H_DIRECT_B_RESET           (1 << 15)

// REG_SOUNDCNT_X
#define GBA_SOUNDCNT_X_GB_1_ACTIVE              (1 << 0)
#define GBA_SOUNDCNT_X_GB_2_ACTIVE              (1 << 1)
#define GBA_SOUNDCNT_X_GB_3_ACTIVE              (1 << 2)
#define GBA_SOUNDCNT_X_GB_4_ACTIVE              (1 << 3)
#define GBA_SOUNDCNT_X_MASTER_ENABLE            (1 << 7)
