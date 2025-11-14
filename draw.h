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
