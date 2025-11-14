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
#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "audio_ctrl.h"

#define STREAM_BYTE_SIZE 8

typedef struct audios_stream_t {
	u_int channels;      /* number of channels on the audio device */
	u_int milliseconds;  /* duration of the stream */
	u_int precision;     /* the number of bits per sample */
	u_int total_size;    /* total memory size of all buffers */
	u_int total_samples; /* total number of samples across all buffers */
	u_int encoding;      /* the encoding of the audio device */
} audio_stream_t;

int build_stream(u_int milliseconds, u_int channels, u_int sample_rate,
    u_int buffer_size, u_int precision, u_int encoding, audio_stream_t *stream);

int build_stream_from_ctrl(audio_ctrl_t ctrl, u_int ms, audio_stream_t *stream);
int stream(audio_ctrl_t ctrl, audio_stream_t stream, u_char *data);
#endif
