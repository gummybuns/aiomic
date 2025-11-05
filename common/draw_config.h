#ifndef AUDIO_DRAW_CONFIG_H
#define AUDIO_DRAW_CONFIG_H

#define PADDING_PCT 0.1f

typedef struct draw_config_t {
	char use_color; /* whether to use colors */
	int rows;      /* number of rows on screen */
	int cols;      /* number of cols on screen */
	int max_h;     /* max height (including padding) */
	int max_w;     /* max width (including padding) */
	int y_padding; /* padding top/bottom */
	int x_padding; /* padding left/right */
	u_int bar_width; /* width of each bar */
	u_int bar_space; /* amount of space between each bar */
	u_int nbars;   /* number of bars to draw in frequency screen */
	short bar_color; /* color to paint inside of each bar */
} draw_config_t;

#endif
