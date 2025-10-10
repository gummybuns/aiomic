#include <curses.h>
#include <math.h>
#include <stdlib.h>

#include "audio_ctrl.h"
#include "audio_rms.h"
#include "audio_stream.h"
#include "draw.h"
#include "error_codes.h"
#include "fft.h"
#include "pcm.h"

/*
 * Print details about the audio controller
 */
static void
print_ctrl(audio_ctrl_t ctrl)
{
	const char *mode, *config_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name(ctrl.config.encoding);

	printw("Audio Controller\n"
	       "\tDevice:\t\t%s\n"
	       "\tMode:\t\t%s\n"
	       "\tConfiguration:\n"
	       "\t\tbuffer_size:\t\t%d\n"
	       "\t\tsample_rate:\t\t%d\n"
	       "\t\tprecision:\t\t%d\n"
	       "\t\tchannels:\t\t%d\n"
	       "\t\tencoding:\t\t%s\n"
	       "\t\tpause:\t\t\t%d\n",
	    ctrl.path, mode, ctrl.config.buffer_size, ctrl.config.sample_rate,
	    ctrl.config.precision, ctrl.config.channels, config_encoding,
	    ctrl.config.pause);
}

/*
 * Check if the user pressed any of the navigation options
 */
static int
check_options(int keypress)
{
	if (keypress == 'I') {
		return DRAW_INFO;
	} else if (keypress == 'R') {
		return DRAW_RECORD;
	} else if (keypress == 'Q') {
		return DRAW_EXIT;
	} else if (keypress == 'F') {
		return DRAW_FREQ;
	} else {
		return 0;
	}
}

int
build_draw_config(draw_config_t *config)
{
	int rows, cols;
	u_int x_padding, y_padding;

	getmaxyx(stdscr, rows, cols);
	x_padding = (u_int)((float)cols * PADDING_PCT);
	y_padding = (u_int)((float)rows * PADDING_PCT);

	config->rows = (u_int)rows;
	config->cols = (u_int)cols;
	config->x_padding = x_padding;
	config->y_padding = y_padding;
	config->max_h = (u_int)rows - y_padding * 2;
	config->max_w = (u_int)cols - x_padding * 2;
	config->bars = config->max_w;

	return 0;
}

/*
 * Display information about the audio controlers + streams
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream)
{
	char keypress;
	int option;

	move(0, 0);
	nodelay(stdscr, FALSE);
	print_ctrl(ctrl);
	for (;;) {
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_INFO) {
			return option;
		}
	}
}

inline int
reset_bars(bar_t *bars, draw_config_t draw_config, fft_config_t fft_config)
{
	u_int i;
	for (i = 0; i < draw_config.bars; i++) {
		float frac_start = (float)i / (float)draw_config.bars;
		float frac_end = (float)(i + 1) / (float)draw_config.bars;
		bars[i].f_min =
		    fft_config.f_min *
		    powf(fft_config.f_max / fft_config.f_min, frac_start);
		bars[i].f_max =
		    fft_config.f_min *
		    powf(fft_config.f_max / fft_config.f_min, frac_end);
		bars[i].magnitude = 0.0f;
		bars[i].bin_count = 0;
	}

	return 0;
}

/*
 * Displays a screen to record audio and display the data in the frequency
 * spectrum.
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_frequency(audio_ctrl_t ctrl, audio_stream_t *audio_stream,
    fft_config_t fft_config, draw_config_t draw_config)
{
	char keypress;
	int draw_start, option, res;
	u_int active_bars, i, j, title_center;
	float pcm[audio_stream->total_samples];
	bar_t bars[draw_config.bars];
	bin_t bins[fft_config.bins];

	title_center = draw_config.cols / 2 - 10;

	mvprintw(0, (int)title_center, "Measure Mic Frequency\n");
	refresh();

	nodelay(stdscr, TRUE);
	for (;;) {
		reset_bins(bins, fft_config);
		reset_bars(bars, draw_config, fft_config);

		if ((res = stream(ctrl, audio_stream)) != 0) {
			return res;
		}

		if ((res = to_normalized_pcm(audio_stream, pcm)) > 0) {
			return res;
		}

		fft(fft_config, bins, pcm);

		/* Attribute a bin to the corresponding bar */
		for (i = 0; i < fft_config.bins; i++) {
			float freq = bins[i].frequency;
			for (j = 0; j < draw_config.bars; j++) {
				if (freq >= bars[j].f_min &&
				    freq < bars[j].f_max) {
					bars[j].magnitude += bins[i].magnitude;
					bars[j].bin_count += 1;
					break;
				}
			}
		}

		active_bars = 0;
		for (i = 0; i < draw_config.bars; i++) {
			/*
			 * Based on the number of bins / number of bars it is
			 * possible that some bars just have no data. We are
			 * going to skip drawing these so there are no gaps
			 * in the bar graph
			 */
			if (bars[i].bin_count <= 0)
				continue;
			active_bars++;
		}

		draw_start = (int)draw_config.x_padding +
			     (int)(draw_config.max_w - active_bars) / 2;
		j = 0;
		for (i = 0; i < draw_config.bars; i++) {
			// TODO it would be great to move this to a separate
			// pane
			// mvprintw((int)i, 0, "%f - %f: %f / %d",
			// bars[i].f_min, bars[i].f_max, bars[i].magnitude,
			// bars[i].bin_count);
			if (bars[i].bin_count <= 0)
				continue;

			float a = bars[i].magnitude / (float)bars[i].bin_count;
			float scaled_magnitude = fminf(ceilf(a * 1.2f),
			    (float)draw_config.max_h -
				(float)draw_config.y_padding);
			mvvline((int)draw_config.y_padding, (int)j + draw_start,
			    ' ', (int)draw_config.max_h);
			mvvline((int)draw_config.max_h - (int)scaled_magnitude,
			    (int)j + draw_start, '|', (int)scaled_magnitude);
			j++;
		}
		refresh();

		/* listen for input */
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_FREQ) {
			return option;
		}
	}
}

/*
 * Displays a screen to record audio and show the intensity based on the
 * Root Mean Square
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_intensity(audio_ctrl_t ctrl, audio_stream_t *audio_stream)
{
	char keypress;
	int title_center;
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	int option;
	int res;
	float rms, percent;

	getmaxyx(stdscr, row, col);
	y_padding = col / 10;
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;
	title_center = col / 2 - 10;

	mvprintw(0, title_center, "Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0%%");
	mvprintw(3 + x_padding, bar_start + bar_distance / 2 - 2, "50%%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100%%");
	refresh();

	nodelay(stdscr, TRUE);

	for (;;) {
		/* record the audio to the stream */
		res = stream(ctrl, audio_stream);
		if (res != 0) {
			return res;
		}

		/* calculate rms */
		rms = calc_rms(audio_stream->data, audio_stream->precision,
		    audio_stream->total_samples);
		percent = calc_rms_percent(rms, audio_stream->precision);

		if (percent < 0) {
			return E_RMS_UNKNOWN_PRECISION;
		}

		/* draw */
		draw_length =
		    (int)((float)bar_distance * (percent / (float)100.0));
		mvhline(2, 0, ' ', col);
		mvprintw(2, title_center + 2, "RMS:\t%d", (int)rms);
		mvhline(2 + x_padding, bar_start, ' ', bar_distance);
		mvhline(2 + x_padding, bar_start, '=', draw_length);
		move(1, 0);
		refresh();

		/* listen for input */
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_RECORD) {
			return option;
		}
	}
}

/*
 * Renders the nav options at the bottom of the screen for the user to see
 */
void
draw_options(void)
{
	int row;
	row = getmaxy(stdscr);
	mvprintw(row - 1, 0, "OPTIONS: ");
	printw("R: RECORD / F: FREQ / I: INFO / Q: QUIT");
	refresh();
}
