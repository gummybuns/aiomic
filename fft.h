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
#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <sys/audioio.h>

#define DEFAULT_NSAMPLES 1024
#define DEFAULT_FMIN 50.0f

typedef double complex cplx;

typedef struct bin_t {
	float frequency;
	float magnitude;
} bin_t;

typedef struct fft_config_t {
	u_int fs;            /* sample rate */
	u_int nbins;         /* number of bins. typically nsamples / 2 */
	u_int nframes;       /* total_samples / nsamples */
	u_int nsamples;      /* number of samples per fft operation */
	u_int total_samples; /* total number of samples to process */
	float fmin;          /* min frequency for the bars */
	float fmax;          /* max frequency for the bars. (fs / 2) */
} fft_config_t;

int fft(fft_config_t config, bin_t *bins, float *pcm);
int build_fft_config(fft_config_t *config, u_int size, u_int fs, u_int total_samples, float f_min);
int reset_bins(bin_t *bins, fft_config_t config);
#endif
