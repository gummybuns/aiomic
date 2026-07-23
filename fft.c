/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Zac Brown.
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
#include "fft.h"
#include "error_codes.h"
#include <math.h>

static void evens(cplx *, cplx *, u_int);
static void odds(cplx *, cplx *, u_int);
static void ct_fft(cplx *, cplx *, u_int);

static void
evens(cplx *in, cplx *out, u_int in_size)
{
	u_int i, j;

	j = 0;
	for (i = 0; i < in_size; i += 2) {
		out[j] = in[i];
		j++;
	}
}

static void
odds(cplx *in, cplx *out, u_int in_size)
{
	if (in_size <= 1) {
		return;
	}

	evens(in + 1, out, in_size - 1);
}

/*
 * Cooley-Tukey FFT Implementation
 */
static void
ct_fft(cplx *A, cplx *Y, u_int N)
{
	u_int j;
	double PI;
	cplx W, WN;
	cplx A_even[N / 2];
	cplx A_odd[N / 2];
	cplx Y_even[N / 2];
	cplx Y_odd[N / 2];

	PI = atan2(1, 1) * 4;

	if (N == 1) {
		Y[0] = A[0];
	} else {
		W = 1;
		WN = cexp(-2.0 * PI * I / N);

		evens(A, A_even, N);
		odds(A, A_odd, N);
		ct_fft(A_even, Y_even, N / 2);
		ct_fft(A_odd, Y_odd, N / 2);

		for (j = 0; j <= N / 2 - 1; j++) {
			Y[j] = Y_even[j] + W * Y_odd[j];
			Y[j + N/2] = Y_even[j] - W * Y_odd[j];
			W = W * WN;
		}
	}
}

/*
 * Perform the fft on the normalized pcm data
 *
 * Breaks the pcm data into subsets (frames) based on the fft_config, each frame
 * frame being a specific size (nsamples).
 *
 * The fft is then calculated on that
 * specific frame, and the magnitude is each frequency bin is summed up and
 * averaged over each frame.
 */
int
fft(struct fft_config config, struct bin *bins, float *pcm)
{
	u_int i, j, start;
	float real, imag;
	cplx buf[config.nsamples];
	cplx out[config.nsamples];

	for (i = 0; i < config.nframes; i++) {
		start = i * config.nsamples;
		for (j = 0; j < config.nsamples; j++) {
			buf[j] = pcm[start + j];
			out[j] = 0;;
		}

		ct_fft(buf, out, config.nsamples);

		for (j = 0; j < config.nbins; j++) {
			real = (float)creal(out[j]);
			imag = (float)cimag(out[j]);
			bins[j].magnitude += sqrtf(real * real + imag * imag);
		}
	}

	for (i = 0; i < config.nbins; i++) {
		bins[i].magnitude = bins[i].magnitude / (float)config.nframes;
	}

	return 0;
}

/*
 * Initialize the fft_config
 */
int
build_fft_config(struct fft_config *config, u_int nsamples, u_int fs,
    u_int total_samples, float fmin)
{

	if (total_samples < nsamples) {
		return E_FFT_CONFIG_TOTAL_SAMPLES;
	}

	if (!(nsamples > 0 && (nsamples & (nsamples - 1)) == 0)) {
		return E_FFT_CONFIG_NSAMPLES_BY_2;
	}

	config->nsamples = nsamples;
	config->nbins = nsamples / 2;
	config->fmin = fmin;
	config->fs = fs;
	config->fmax = (float)fs / 2.0f;
	config->total_samples = total_samples;
	config->nframes = total_samples / nsamples;

	return 0;
}

/*
 * Reset the bins to their initial values
 */
int
reset_bins(struct bin *bins, struct fft_config config)
{
	u_int i;

	for (i = 0; i < config.nbins; i++) {
		bins[i].magnitude = 0.0f;
		bins[i].frequency =
		    (float)i * (float)config.fs / (float)config.nsamples;
	}

	return 0;
}
