#include "fft.h"
#include <math.h>
#include "error_codes.h"

/*
 * Helper function to do the fft math
 */
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
			buf[i / 2] = out[i] + t;
			buf[(i + n) / 2] = out[i] - t;
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
fft(fft_config_t config, bin_t *bins, float *pcm)
{
	u_int i, j, start;
	float real, imag;
	cplx buf[config.nsamples];
	cplx out[config.nsamples];

	for (i = 0; i < config.nframes; i++) {
		start = i * config.nsamples;
		for (j = 0; j < config.nsamples; j++) {
			buf[j] = pcm[start + j];
			out[j] = pcm[start + j];
		}

		_fft(buf, out, config.nsamples, 1);

		for (j = 0; j < config.nbins; j++) {
			real = (float)creal(buf[j]);
			imag = (float)cimag(buf[j]);
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
build_fft_config(fft_config_t *config, u_int nsamples, u_int fs, u_int total_samples, float fmin)
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
reset_bins(bin_t *bins, fft_config_t config)
{
	u_int i;

	for (i = 0; i < config.nbins; i++) {
		bins[i].magnitude = 0.0f;
		bins[i].frequency =
		    (float)i * (float)config.fs / (float)config.nsamples;
	}

	return 0;
}
