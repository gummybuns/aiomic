/*
mode: 2 enc: 6  bits: 16        prec: 16        channels: 1     mask: 0 freq: {16000,}  priority: 0
mode: 2 enc: 6  bits: 16        prec: 16        channels: 1     mask: 0 freq: {24000,}  priority: 0
mode: 2 enc: 6  bits: 16        prec: 16        channels: 1     mask: 0 freq: {32000,}  priority: 0
mode: 2 enc: 6  bits: 16        prec: 16        channels: 1     mask: 0 freq: {48000,}  priority: 0
*/
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
#define DEFAULT_BAR_WIDTH 4
#define DEFAULT_BOX_HEIGHT 2

static inline int
build_draw_config(draw_config_t *config)
{
	int rows, cols, x_padding, y_padding;
	u_int computed;

	getmaxyx(stdscr, rows, cols);
	x_padding = (int)((float)cols * PADDING_PCT);
	y_padding = (int)((float)rows * PADDING_PCT);

	config->rows = rows;
	config->cols = cols;
	config->x_padding = x_padding;
	config->y_padding = y_padding;
	config->max_h = rows - y_padding * 2;
	config->max_w = cols - x_padding * 2;

	if (config->bar_width == 0) {
		config->bar_width = DEFAULT_BAR_WIDTH;
	}

	if (config->nbars == 0) {
		config->nbars = (u_int)config->max_w/(config->bar_width + config->bar_space);

		if (config->nbars <= 0) {
			return E_DRW_CONFIG_NBARS_ZERO;
		}
	}

	if (config->use_boxes) {
		config->nboxes = (u_int)config->max_h/(config->box_height + config->box_space);
		if (config->nboxes <= 0) {
			return E_DRW_CONFIG_NBOXES_ZERO;
		}
	} else {
		config->nboxes = 1;
		config->box_height = config->max_h / (config->nboxes + config->box_space);
	}

	computed = config->box_height * config->nboxes + config->box_space * config->nboxes;
	if (computed > config->max_h) {
		return E_DRW_CONFIG_NBOXES;
	}

	computed = config->nbars * config->bar_width + config->nbars * config->bar_space;
	if (computed > (u_int)config->max_w) {
		return E_DRW_CONFIG_NBARS;
	}

	if (config->box_height <= 0) {
		return E_DRW_CONFIG_BOX_HEIGHT_ZERO;
	}

	if (config->use_color && config->use_boxes && config->bar_color2 >= 0) {
		config->ncolors = (u_int)fminf((float)config->nboxes, COLOR_PAIRS - 1);
	}

	return 0;
}

static const char * shortopts = "c:d:e:f:m:p:s:H:N:W:C:E:M:S:XU";
static struct option longopts[] = {
	{ "channels", 		required_argument, 	NULL,	'c' },
	{ "device", 		required_argument, 	NULL,	'd' },
	{ "encoding",		required_argument,	NULL,	'e' },
	{ "fft-samples",	required_argument,	NULL,	'f' },
	{ "fft-fmin",		required_argument,	NULL,	'm' },
	{ "precision",		required_argument,	NULL,	'p' },
	{ "sample-rate",	required_argument,	NULL,	's' },
	{ "color",			required_argument,	NULL,	'C' },
	{ "color-end",		required_argument,	NULL,	'E' },
	{ "box-height",		required_argument,	NULL,	'H' },
	{ "milliseconds",	required_argument,	NULL,	'M' },
	{ "num-bars",		required_argument,	NULL,	'N' },
	{ "box-space",		required_argument,	NULL,	'S' },
	{ "use-colors",		no_argument,		NULL,	'U' },
	{ "use-boxes",		no_argument,		NULL,	'X' },
	{ "bar-width",		required_argument,	NULL,	'W' },
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
	draw_config.bar_width = 0;
	draw_config.bar_color = -1;
	draw_config.bar_color2 = -1;
	draw_config.bar_space = 0;
	draw_config.use_color = 0;
	draw_config.use_boxes = 0;
	draw_config.box_space = 1;
	draw_config.box_height = DEFAULT_BOX_HEIGHT;
	draw_config.ncolors = 0;
	fft_samples = DEFAULT_NSAMPLES;
	fft_fmin = DEFAULT_FMIN;
	ms = DEFAULT_STREAM_DURATION;

	while ((ch = getopt_long(argc, argv,shortopts, longopts, NULL)) != -1) {
		switch (ch) {
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
		case 'p':
			decode_uint(optarg, &(audio_config.precision));
			break;
		case 's':
			decode_uint(optarg, &(audio_config.sample_rate));
			break;
		case 'H':
			decode_uint(optarg, &(draw_config.box_height));
			break;
		case 'N':
			decode_uint(optarg, &(draw_config.nbars));
			break;
		case 'C':
			draw_config.use_color = 1;
			decode_color(optarg, &(draw_config.bar_color));
			draw_config.bar_space = 1;
			draw_config.ncolors = 1;
			break;
		case 'E':
			draw_config.use_color = 1;
			decode_color(optarg, &(draw_config.bar_color2));
			draw_config.bar_space = 1;
			break;
		case 'M':
			decode_uint(optarg, &ms);
			break;
		case 'S':
			decode_uint(optarg, &(draw_config.box_space));
			break;
		case 'U':
			draw_config.use_color = 1;
			draw_config.bar_space = 1;
			draw_config.ncolors = 1;
			break;
		case 'X':
			draw_config.use_boxes = 1;
			break;
		case 'W':
			decode_uint(optarg, &(draw_config.bar_width));
			break;
		default:
			// TODO - usage()
			err(1, "%c is invalid argument", (char)ch);
		}
	}

	if (initscr() == NULL) {
		err(1, "can't initialize curses");
	}

	cbreak();
	noecho();
	curs_set(0);

	if ((res = build_audio_ctrl(&rctrl, path, AUMODE_RECORD)) != 0) {
		goto handle_error;
	}

	if ((res = update_audio_ctrl(&rctrl, audio_config)) != 0) {
		goto handle_error;
	}

	if ((res = build_stream_from_ctrl(rctrl, ms, &rstream)) != 0) {
		goto handle_error;
	}

	option = DRAW_INFO;

	res = build_fft_config(&fft_config, fft_samples, rctrl.config.sample_rate, rstream.total_samples, (float)fft_fmin);
	if (res != 0) {
		goto handle_error;
	}

	if ((res = build_draw_config(&draw_config)) != 0) {
		goto handle_error;
	}

	// TODO - this is dumb but you need to start-color before you build
	// draw config because it depends on some methods in there
	if (draw_config.use_color) {
		if (!has_colors()) {
			res = E_NO_COLORS;
			goto handle_error;
		}
		start_color();
	}
	build_draw_config(&draw_config);

	if (draw_config.use_color) {
		use_default_colors();
		if (draw_config.ncolors <= 1) {
			init_pair(1, draw_config.bar_color, -1);
		} else {
			if (!can_change_color()) {
				res = E_CHANGE_COLORS;
				goto handle_error;
			}
			short r1, g1, b1;
			short r2, g2, b2;
			short r_interp, g_interp, b_interp;
			float t;

			/* TODO - this is working but it is actually changing
			 * the terminal colors? like when i do a git status
			 * the diff is now blue instead of red?
			 *
			 * do i have to clean up the init_colors somehow?
			 */
			color_content(draw_config.bar_color, &r1, &g1, &b1);
			color_content(draw_config.bar_color2, &r2, &g2, &b2);
			for (int c = 0; c < draw_config.ncolors; c++) {
				t = (float)c / (float)(draw_config.ncolors - 1);
				r_interp = (int)((1-t) * r1 + t * r2);
				g_interp = (int)((1-t) * g1 + t * g2);
				b_interp = (int)((1-t) * b1 + t * b2);
				if((res = init_color(c, r_interp, g_interp, b_interp)) != 0) {
					res = E_INIT_COLOR;
					goto handle_error;
				}
				printf("%d - t=%f - %d / %d / %d\n", c, t, r_interp, g_interp, b_interp);
				init_pair(c, c, -1);
			}
		}
	}

	for (;;) {
		draw_options();

		if (option >= E_UNHANDLED) {
			res = option;
			goto handle_error;
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
handle_error:
	endwin();
	errc(EXIT_FAILURE, res, get_error_msg(res));
}
