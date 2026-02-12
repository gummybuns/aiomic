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
#define STREAM_BYTE_SIZE 8

struct audio_config {
	u_int buffer_size; /* size of the audio device buffer in bytes */
	u_int channels;    /* number of channels for the audio device */
	u_int encoding;    /* the encoding of the audio device */
	u_int precision;   /* the number of bits per sample */
	u_int sample_rate; /* number of samples per second */
};

struct stream {
	u_int ms;			/* number of milliseconds to stream */
	u_int total_size;	/* the total size in bytes of the resulting buffer */
	u_int total_samples; /* the number of samples in the resulting buffer */
	u_int rate; /* the incremental size to read/write the audio device */
};

struct audio_ctrl {
	int fd;                     /* file descriptor to the audio device */
	u_int mode;                 /* record vs play */
	struct audio_config config; /* the configuration of the audio device */
	struct stream stream;
	const char *path; /* the path to the audio device */
};

int build_audio_ctrl(struct audio_ctrl *, const char *, u_int, u_int);
int update_audio_ctrl(struct audio_ctrl *, struct audio_config);
char is_pad_device(const char *);
const char *get_encoding_name(u_int);
const char *get_mode(struct audio_ctrl);
int stream(struct audio_ctrl *, u_char *);
#endif
