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
