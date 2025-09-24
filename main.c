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

#define STREAM_DURATION 250

int
main(int argc, char *argv[])
{
	int option;
	int res;
	char *path;
	audio_ctrl_t rctrl;
	audio_stream_t rstream;

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

	initscr();
	raw();
	noecho();

	option = DRAW_RECORD;
	for (;;) {
		draw_options();

		if (option >= E_UNHANDLED) {
			err(1, "Unhandled Error: %d", option);
		}

		if (option == DRAW_RECORD) {
			res = build_stream_from_ctrl(rctrl, STREAM_DURATION,
			    &rstream);
			if (res != 0) {
				err(1, "Failed to build audio stream: %d", res);
			}

			option = draw_intensity(rctrl, &rstream);
			clean_buffers(&rstream);
		} else if (option == DRAW_INFO) {
			option = draw_info(rctrl, rstream);
		} else if(option == DRAW_FREQ) {
			res = build_stream_from_ctrl(rctrl, STREAM_DURATION,
			    &rstream);
			if (res != 0) {
				err(1, "Failed to build audio stream: %d", res);
			}
			option = draw_frequency(rctrl);
			clean_buffers(&rstream);
		} else {
			break;
		}
		clear();
	}

	endwin();
	return 0;
}
