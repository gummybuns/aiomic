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
#ifndef AUDIO_DRAW_H
#define AUDIO_DRAW_H

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "draw_config.h"
#include "fft.h"

#define DRAW_EXIT -1
#define DRAW_RECORD 1
#define DRAW_INFO 2
#define DRAW_FREQ 3
#define DRAW_DEBUG 4

#define FREQ_SCALE_FACTOR 1.2f
#define DEFAULT_BAR_COUNT 50

#define PADDING_PCT 0.1f

typedef struct bar_t {
	float fmin;  /* minimum frequency of the bar */
	float fmax;  /* maximum frequency of the bar */
	u_int nbins; /* number of bins represented in the bar */
	float
	    magnitude; /* sum of all amplitudes of all bins within frequency */
} bar_t;

int draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream, fft_config_t fft_config, draw_config_t draw_config);
int draw_frequency(audio_ctrl_t ctrl, audio_stream_t audio_stream,
    fft_config_t fft_config, draw_config_t draw_config);
#endif
