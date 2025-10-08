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
#include "error_codes.h"
#include "fft.h"

#define STREAM_DURATION 250

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

	res = build_audio_ctrl(&rctrl, path, CTRL_RECORD);
	if (res != 0) {
		err(1, "Failed to build record audio controller: %d", res);
	}

	res = build_stream_from_ctrl(rctrl, STREAM_DURATION, &rstream);
	if (res != 0) {
		err(1, "Failed to build audio stream: %d", res);
	}


	initscr();
	raw();
	noecho();

	option = DRAW_RECORD;

	build_fft_config(&fft_config, DEFAULT_FFT_SIZE, DEFAULT_BIN_SIZE, rctrl.config.sample_rate, rstream.total_samples, DEFAULT_F_MIN);
	build_draw_config(&draw_config);

	for (;;) {
		draw_options();

		if (option >= E_UNHANDLED) {
			err(1, "Unhandled Error: %d", option);
		}

		if (option == DRAW_RECORD) {
			option = draw_intensity(rctrl, &rstream);
		} else if (option == DRAW_INFO) {
			option = draw_info(rctrl, rstream);
		} else if(option == DRAW_FREQ) {
			option = draw_frequency(rctrl, &rstream, fft_config, draw_config);
		} else {
			break;
		}
		clear();
	}

	clean_buffers(&rstream);
	endwin();
	return 0;
}
