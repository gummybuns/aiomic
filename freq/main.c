#include <sys/audioio.h>

#include <curses.h>
#include <err.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "draw.h"
#include "draw_config.h"
#include "error_codes.h"
#include "fft.h"

#define STREAM_DURATION 250

static inline int
build_draw_config(draw_config_t *config)
{
	int rows, cols, x_padding, y_padding;

	getmaxyx(stdscr, rows, cols);
	x_padding = (int)((float)cols * PADDING_PCT);
	y_padding = (int)((float)rows * PADDING_PCT);

	config->rows = rows;
	config->cols = cols;
	config->x_padding = x_padding;
	config->y_padding = y_padding;
	config->max_h = rows - y_padding * 2;
	config->max_w = cols - x_padding * 2;
	config->nbars = (u_int)config->max_w;

	return 0;
}

int
main(int argc, char *argv[])
{
	int option;
	int res;
	char *path;
	audio_ctrl_t rctrl;
	audio_stream_t rstream;
	fft_config_t fft_config;
	draw_config_t draw_config;

	setprogname(argv[0]);

	if (argc <= 1) {
		errno = EINVAL;
		err(1, "Specify an audio device");
	}

	path = argv[1];

	res = build_audio_ctrl(&rctrl, path, AUMODE_RECORD);
	if (res != 0) {
		err(1, "Failed to build record audio controller: %d", res);
	}

	res = build_stream_from_ctrl(rctrl, STREAM_DURATION, &rstream);
	if (res != 0) {
		err(1, "Failed to build audio stream: %d", res);
	}

	if (initscr() == NULL) {
		err(1, "can't initialize curses");
	}
	cbreak();
	noecho();
	curs_set(0);

	option = DRAW_FREQ;

	build_fft_config(&fft_config, DEFAULT_NSAMPLES, DEFAULT_NBINS,
	    rctrl.config.sample_rate, rstream.total_samples, DEFAULT_FMIN);
	build_draw_config(&draw_config);

	printf("BEFORE FOR LOOP\n\n");
	for (;;) {
		draw_options();

		if (option >= E_UNHANDLED) {
			err(1, "Unhandled Error: %d", option);
		}

		if (option == DRAW_INFO) {
			option = draw_info(rctrl, rstream);
		} else if (option == DRAW_FREQ) {
			option = draw_frequency(rctrl, rstream, fft_config,
			    draw_config);
		} else {
			break;
		}
		clear();
	}

	endwin();
	return 0;
}
