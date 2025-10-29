#include <sys/audioio.h>

#include <curses.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "decode.h"
#include "draw.h"
#include "draw_config.h"
#include "error_codes.h"
#include "fft.h"

#define DEFAULT_STREAM_DURATION 250
#define DEFAULT_PATH "/dev/sound"

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

	if (config->nbars == 0) {
		config->nbars = (u_int)config->max_w;
	}

	if (config->nbars > (u_int)config->max_w) {
		return E_DRW_CONFIG_NBARS;
	}

	return 0;
}

static const char * shortopts = "b:c:d:e:f:m:n:p:s:M:";
static struct option longopts[] = {
	{ "buffer-size", 	required_argument,	NULL, 	'b' },
	{ "channels", 		required_argument, 	NULL,	'c' },
	{ "device", 		required_argument, 	NULL,	'd' },
	{ "encoding",		required_argument,	NULL,	'e' },
	{ "fft-samples",	required_argument,	NULL,	'f' },
	{ "num-bars",		required_argument,	NULL,	'n' },
	{ "fft-fmin",		required_argument,	NULL,	'm' },
	{ "precision",		required_argument,	NULL,	'p' },
	{ "sample-rate",	required_argument,	NULL,	's' },
	{ "milliseconds",	required_argument,	NULL,	'M' }
};

int
main(int argc, char *argv[])
{
	int ch, option, res;
	u_int fft_samples, fft_fmin, ms;
	const char *path;
	audio_ctrl_t rctrl;
	audio_config_t audio_config;
	audio_stream_t rstream;
	fft_config_t fft_config;
	draw_config_t draw_config;

	setprogname(argv[0]);

	path = DEFAULT_PATH;

	audio_config.buffer_size = 0;
	audio_config.channels = 0;
	audio_config.encoding = 0;
	audio_config.precision = 0;
	audio_config.sample_rate = 0;
	draw_config.nbars = 0;
	fft_samples = DEFAULT_NSAMPLES;
	fft_fmin = DEFAULT_FMIN;
	ms = DEFAULT_STREAM_DURATION;

	while ((ch = getopt_long(argc, argv,shortopts, longopts, NULL)) != -1) {
		switch (ch) {
		case 'b':
			// TODO figure this out
			audio_config.buffer_size = (u_int)strsuftoll("read buffer size", optarg, 1, UINT_MAX);
			break;
		case 'c':
			decode_uint(optarg, &(audio_config.channels));
			break;
		case 'd':
			path = optarg;
			break;
		case 'e':
			// TODO figure out encoding strings next
			break;
		case 'f':
			decode_uint(optarg, &fft_samples);
			break;
		case 'm':
			decode_uint(optarg, &fft_fmin);
			break;
		case 'n':
			decode_uint(optarg, &(draw_config.nbars));
			break;
		case 'p':
			decode_uint(optarg, &(audio_config.precision));
			break;
		case 's':
			decode_uint(optarg, &(audio_config.sample_rate));
			break;
		case 'M':
			decode_uint(optarg, &ms);
			break;
		default:
			// TODO - usage()
			err(1, "%c is invalid argument", (char)ch);
		}
	}

	res = build_audio_ctrl(&rctrl, path, AUMODE_RECORD);
	if (res != 0) {
		err(1, "Failed to build record audio controller: %d", res);

	}

	res = update_audio_ctrl(&rctrl, audio_config);
	if (res != 0) {
		err(1, "Failed to set audio controller: %d", res);
	}

	res = build_stream_from_ctrl(rctrl, ms, &rstream);
	if (res != 0) {
		err(1, "Failed to build audio stream: %d", res);
	}

	if (initscr() == NULL) {
		err(1, "can't initialize curses");
	}
	cbreak();
	noecho();
	curs_set(0);

	option = DRAW_INFO;

	// TODO - now that there is user input it is posible to get nans
	// this happens when the fft_config.nsamples is less than the
	// fft_config.total_samples (which is from the audio_stream)
	// ./aiofreq -s 4092 shows this. we get 1023 total_samples but
	// DEFAULT_NSAMPLES is 1024
	// TOTAL_SAMPLES MUST BE BIGGER THAN NSAMPLES
	// NSAMPLES MUST BE A POWER OF 2
	// ex - ./freq/aiofreq --sample-rate 4000 --fft-samples 2048
	// but there is some nuance still to figure out because:
	// ./freq/aiofreq --sample-rate 4000 --fft-samples 1000
	// causes a segfault. in fact fft-samples for any number less than 1000
	// causes a segfault, and any number greater than 1000 causes nans
	res = build_fft_config(&fft_config, fft_samples, rctrl.config.sample_rate, rstream.total_samples, (float)fft_fmin);
	if (res != 0) {
		err(1, "Failed to initialize fft_config: %d", res);
	}

	if ((res = build_draw_config(&draw_config)) != 0) {
		err(1, "Failed to initialize draw_config: %d", res);
	}
	build_draw_config(&draw_config);

	for (;;) {
		draw_options();

		if (option >= E_UNHANDLED) {
			err(1, "Unhandled Error: %d", option);
		}

		if (option == DRAW_INFO) {
			option = draw_info(rctrl, rstream, fft_config);
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
