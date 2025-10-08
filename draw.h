#ifndef AUDIO_DRAW_H
#define AUDIO_DRAW_H

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "fft.h"

#define DRAW_EXIT -1
#define DRAW_RECORD 1
#define DRAW_INFO 2
#define DRAW_FREQ 3

#define DEFAULT_BAR_COUNT 50

#define PADDING_PCT 0.1f

typedef struct bar_t {
	float f_min;	/* minimum frequency of the bar */
	float f_max;	/* maximum frequency of the bar */
	int bin_count;	/* number of bins represented in the bar */
	float magnitude;/* sum of all amplitudes of all bins within frequency */
} bar_t;

typedef struct draw_config_t {
	int rows;	/* number of rows on screen */
	int cols;	/* number of cols on screen */
	int max_h;	/* max height (including padding) */
	int max_w;	/* max width (including padding) */
	int y_padding;	/* padding top/bottom */
	int x_padding;	/* padding left/right */
	int bars;	/* number of bars to draw in frequency screen */
} draw_config_t;

int build_draw_config(draw_config_t *config);
int draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream);
int draw_intensity(audio_ctrl_t rctrl, audio_stream_t *audio_stream);
int draw_frequency(audio_ctrl_t ctrl, audio_stream_t *audio_stream, fft_config_t fft_config, draw_config_t draw_config);
void draw_options(void);
#endif
