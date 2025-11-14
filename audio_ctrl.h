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
#ifndef AUDIO_CTRL_H
#define AUDIO_CTRL_H

#include <sys/audioio.h>

#define CTRL_CFG_PAUSE 1
#define CTRL_CFG_PLAY 0

typedef struct audio_config_t {
	u_int buffer_size; /* size of the audio device buffer in bytes */
	u_int channels;    /* number of channels for the audio device */
	u_int encoding;    /* the encoding of the audio device */
	u_int precision;   /* the number of bits per sample */
	u_int sample_rate; /* number of samples per second */
} audio_config_t;

typedef struct audio_ctrl_t {
	int fd;                /* file descriptor to the audio device */
	u_int mode;            /* record vs play */
	audio_config_t config; /* the configuration of the audio device */
	const char *path;            /* the path to the audio device */
} audio_ctrl_t;

int build_audio_ctrl(audio_ctrl_t *ctrl, const char *path, u_int mode);
int update_audio_ctrl(audio_ctrl_t *ctrl, audio_config_t config);
const char *get_encoding_name(u_int encoding);
const char *get_mode(audio_ctrl_t ctrl);

#endif
