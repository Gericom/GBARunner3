#pragma once

extern "C" void ic_invalidateAll(void);
extern "C" void dc_drainWriteBuffer(void);
extern "C" void dc_flushRange(void* start, u32 count);
