#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <sys/audioio.h>

#define DEFAULT_FFT_SIZE 1024
#define DEFAULT_BIN_SIZE DEFAULT_FFT_SIZE / 2
#define DEFAULT_F_MIN 50.0f

typedef double complex cplx;

typedef struct bin_t {
	float frequency;
	float magnitude;
} bin_t;

typedef struct fft_config_t {
	u_int total_samples;	/* total number of samples to process */
	u_int size;		/* number of samples per fft operation */
	u_int frames;		/* total_samples / size */
	u_int bins;		/* number of bins. typically size / 2 */
	u_int fs;		/* sample rate */
	float f_min;		/* min frequency for the bars */
	float f_max;		/* max frequency for the bars. (fs / 2) */
} fft_config_t;

void fft(cplx *buf, u_int n);
int build_fft_config(fft_config_t *config, u_int size, u_int bins, u_int fs, u_int total_samples, float f_min);
int reset_bins(bin_t *bins, fft_config_t config);
#endif
