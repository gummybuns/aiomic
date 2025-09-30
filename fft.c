#include "fft.h"
#include <math.h>

inline void _fft(cplx *buf, cplx *out, u_int n, u_int step)
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
 
inline void fft(cplx *buf, u_int n)
{
	u_int i;
	cplx out[n];
	for (i = 0; i < n; i++) {
		out[i] = buf[i];
	}
	 
	_fft(buf, out, n, 1);
}
