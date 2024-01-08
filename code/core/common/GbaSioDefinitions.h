#pragma once

// REG_SIOCNT
#define GBA_SIOCNT_CLOCK_SOURCE_EXTERN      (0 << 0)
#define GBA_SIOCNT_CLOCK_SOURCE_INTERNAL    (1 << 0)

#define GBA_SIOCNT_INTERNAL_CLOCK_256_KHZ   (0 << 1)
#define GBA_SIOCNT_INTERNAL_CLOCK_2_MHZ     (1 << 1)

#define GBA_SIOCNT_SI                       (1 << 2)
#define GBA_SIOCNT_NORMAL_SO                (1 << 3)
#define GBA_SIOCNT_ENABLE                   (1 << 7)

#define GBA_SIOCNT_MODE_NORMAL_8            (0 << 12)
#define GBA_SIOCNT_MODE_NORMAL_32           (1 << 12)
#define GBA_SIOCNT_MODE_MULTI               (2 << 12)
#define GBA_SIOCNT_MODE_UART                (3 << 12)
#define GBA_SIOCNT_MODE_MASK                (3 << 12)

#define GBA_SIOCNT_IRQ                      (1 << 14)

#define GBA_SIOCNT_MULTI_SD                 (1 << 3)
#define GBA_SIOCNT_MULTI_ID_MASK            (3 << 4)
#define GBA_SIOCNT_MULTI_ERROR              (1 << 6)

// REG_RCNT
#define GBA_RCNT_SC_VALUE                   (1 << 0)
#define GBA_RCNT_SD_VALUE                   (1 << 1)
#define GBA_RCNT_SI_VALUE                   (1 << 2)
#define GBA_RCNT_SO_VALUE                   (1 << 3)

#define GBA_RCNT_SC_OUTPUT                  (1 << 4)
#define GBA_RCNT_SD_OUTPUT                  (1 << 5)
#define GBA_RCNT_SI_OUTPUT                  (1 << 6)
#define GBA_RCNT_SO_OUTPUT                  (1 << 7)

#define GBA_RCNT_SI_IRQ                     (1 << 8)

#define GBA_RCNT_MODE_GPIO                  (2 << 14)
#define GBA_RCNT_MODE_JOYBUS                (3 << 14)
#define GBA_RCNT_MODE_MASK                  (3 << 14)