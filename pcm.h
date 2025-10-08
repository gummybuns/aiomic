#ifndef PCM_H
#define PCM_H

#include "audio_stream.h"

typedef struct pcm_converter_t {
	void (*swap_func) (u_char *);	/* le <> be - can be null */
	void (*sign_func) (u_char *);	/* unsigned -> signed - can be null */
	float (*normalize_func) (u_char *); /* normalize to a float to 0-1 */
} pcm_converter_t;

int build_converter(pcm_converter_t *converter, u_int prec, u_int enc);
int to_normalized_pcm(void *in, float *out, audio_stream_t *a_stream);

#endif
