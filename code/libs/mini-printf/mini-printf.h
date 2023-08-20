/*
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __MINI_PRINTF__
#define __MINI_PRINTF__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#ifdef MINI_PRINTF_ENABLE_OBJECTS
/* If enabled, callback for object types (O and R).
 * void* arguments matching %O and %R are sent to handler as obj.
 * the result string created by handler at *buf is freed by freeor.
 * */
void mini_printf_set_handler(
	void * data,
	/* handler returns number of chars in *buf; *buf is not NUL-terminated. */
	int (*handler)(void* data, void* obj, int ch, int len_hint, char** buf),
	void (*freeor)(void* data, void* buf));
#endif

/* String IO interface; returns number of bytes written, not including the ending NUL.
 * Always appends a NUL at the end, therefore buffer_len shall be at least 1 in normal operation.
 * If buffer is NULL or buffer_len is 0, returns number of bytes to be written, not including the ending NUL.
 */
int mini_vsnprintf(char* buffer, unsigned int buffer_len, const char *fmt, va_list va);
int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

/* Stream IO interface; returns number of bytes written.
 * If puts is NULL, number of bytes to be written.
 * puts shall return number of bytes written.
 */
int mini_vpprintf(int (*puts)(char* s, int len, void* buf), void* buf, const char *fmt, va_list va);
int mini_pprintf(int (*puts)(char*s, int len, void* buf), void* buf, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#define vsnprintf mini_vsnprintf
#define snprintf mini_snprintf

#endif
