#include "fft.h"
#include <math.h>

inline void
_fft(cplx *buf, cplx *out, u_int n, u_int step)
{
	u_int i;
	double PI;

	PI = atan2(1, 1) * 4;

	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
		 
		for (i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

int
fft(fft_config_t config, bin_t *bins, float *pcm)
{
	u_int i, j, start;
	float real, imag;
	cplx buf[config.size];
	cplx out[config.size];

	for (i = 0; i < config.frames; i++) {
		start = i * config.size;
		for (j = 0; j < config.size; j++) {
			buf[j] = pcm[start+j];
			out[j] = pcm[start+j];
		}

		_fft(buf, out, config.size, 1);

		for (j = 0; j < config.bins; j++) {
			real = (float)creal(buf[j]);
			imag = (float)cimag(buf[j]);
			bins[j].magnitude += sqrtf(real * real + imag * imag);
		}
	}

	for (i = 0; i < config.bins; i++) {
		bins[i].magnitude = bins[i].magnitude / (float) config.frames;
	}

	return 0;
}

int
build_fft_config(fft_config_t  *config, u_int size, u_int bins, u_int fs, u_int total_samples, float f_min)
{
	config->size = size;
	config->bins = bins;
	config->f_min = f_min;
	config->fs = fs;
	config->f_max = (float)fs / 2.0f;
	config->total_samples = total_samples;
	config->frames = total_samples / size;

	return 0;
}

int
reset_bins(bin_t *bins, fft_config_t config)
{
	u_int i;

	for (i = 0; i < config.bins; i++) {
		bins[i].magnitude = 0.0f;
		bins[i].frequency = (float)i * (float) config.fs / (float) config.size;
	}

	return 0;
}

