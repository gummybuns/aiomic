#ifndef AUDIO_DRAW_CONFIG_H
#define AUDIO_DRAW_CONFIG_H

#define PADDING_PCT 0.1f

typedef struct draw_config_t {
	char use_color; /* whether to use colors */
	char use_boxes; /* whether to render a bar of boxes */
	int rows;      /* number of rows on screen */
	int cols;      /* number of cols on screen */
	int max_h;     /* max height (including padding) */
	int max_w;     /* max width (including padding) */
	int y_padding; /* padding top/bottom */
	int x_padding; /* padding left/right */
	u_int bar_width; /* width of each bar */
	u_int bar_space; /* amount of space between each bar */
	u_int nbars;   /* number of bars to draw in frequency screen */
	u_int nboxes; /* number of boxes per bar */
	u_int box_space; /* amount of space between each box */
	u_int box_height; /* height of each box */
	u_int ncolors;	/* total number of colors */
	short bar_color; /* color to paint inside of each bar */
	short bar_color2; /* second color to transition to */
} draw_config_t;

#endif
