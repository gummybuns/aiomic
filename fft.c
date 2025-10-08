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
 
inline void
fft(cplx *buf, u_int n)
{
	u_int i;
	cplx out[n];
	for (i = 0; i < n; i++) {
		out[i] = buf[i];
	}
	 
	_fft(buf, out, n, 1);
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
