#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void ic_invalidateAll(void);
extern void dc_drainWriteBuffer(void);
extern void dc_flushRange(void* start, u32 count);

#ifdef __cplusplus
}
#endif
