/*	$NetBSD: auconv.h,v 1.5.54.1 2024/03/12 12:47:40 martin Exp $	*/

/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/audioio.h>
#include <sys/types.h>

/* Convert between signed and unsigned. */
static inline void change_sign8(u_char *);
static inline void change_sign16_le(u_char *);
static inline void change_sign16_be(u_char *);
static inline void change_sign32_le(u_char *);
static inline void change_sign32_be(u_char *);
/* Convert between little and big endian. */
static inline void swap_bytes(u_char *);
static inline void swap_bytes32(u_char *);
/* Normalization */
static inline float normalize8(u_char *);
static inline float normalize16(u_char *);
static inline float normalize32(u_char *);

static inline void
change_sign8(u_char *p)
{
	*p ^= 0x80;
}

static inline void
change_sign16_le(u_char *p)
{
	p[1] ^= 0x80;
}

static inline void
change_sign16_be(u_char *p)
{
	p[0] ^= 0x80;
}

static inline void
change_sign32_le(u_char *p)
{
	p[3] ^= 0x80;
}

static inline void
change_sign32_be(u_char *p)
{
	p[0] ^= 0x80;
}

static inline void
swap_bytes(u_char *p)
{
	u_char t;
	t = p[0];
	p[0] = p[1];
	p[1] = t;
}

static inline void
swap_bytes32(u_char *p)
{
	u_char t;
	t = p[0];
	p[0] = p[3];
	p[3] = t;
	t = p[1];
	p[1] = p[2];
	p[2] = t;
	p += 4;
}

static inline float
normalize8(u_char *p)
{
	return (float)p[0] / 128.0f;
}

static inline float
normalize16(u_char *p)
{
	short *s;

	s = (short *)p;
	return (float)s[0] / 32768.0f;
}

static inline float
normalize32(u_char *p)
{
	float *f;

	f = (float *)p;
	return f[0] / 2147483647.0f;
}
