#ifndef AUDIO_DRAW_CONFIG_H
#define AUDIO_DRAW_CONFIG_H

#define PADDING_PCT 0.1f

typedef struct draw_config_t {
	int rows;      /* number of rows on screen */
	int cols;      /* number of cols on screen */
	int max_h;     /* max height (including padding) */
	int max_w;     /* max width (including padding) */
	int y_padding; /* padding top/bottom */
	int x_padding; /* padding left/right */
	u_int nbars;   /* number of bars to draw in frequency screen */
} draw_config_t;

#endif
