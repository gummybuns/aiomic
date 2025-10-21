#include <curses.h>
#include <math.h>
#include <stdlib.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "draw.h"
#include "draw_config.h"
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
	       "\t\tencoding:\t\t%s\n",
	    ctrl.path, mode, ctrl.config.buffer_size, ctrl.config.sample_rate,
	    ctrl.config.precision, ctrl.config.channels, config_encoding);
}

/*
 * Check if the user pressed any of the navigation options
 */
static int
check_options(int keypress)
{
	if (keypress == 'I') {
		return DRAW_INFO;
	} else if (keypress == 'Q') {
		return DRAW_EXIT;
	} else if (keypress == 'F') {
		return DRAW_FREQ;
	} else {
		return 0;
	}
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

/*
 * Reset the bars back to their initial states
 *
 * Each bar is logarithmically spaced apart, meaning the frequency range of the
 * bar increases with each one. This should provide more granular detail for
 * the human audio spectrum.
 */
inline int
reset_bars(bar_t *bars, draw_config_t draw_config, fft_config_t fft_config)
{
	u_int i;
	for (i = 0; i < draw_config.nbars; i++) {
		float frac_start = (float)i / (float)draw_config.nbars;
		float frac_end = (float)(i + 1) / (float)draw_config.nbars;
		bars[i].fmin =
		    fft_config.fmin *
		    powf(fft_config.fmax / fft_config.fmin, frac_start);
		bars[i].fmax =
		    fft_config.fmin *
		    powf(fft_config.fmax / fft_config.fmin, frac_end);
		bars[i].magnitude = 0.0f;
		bars[i].nbins = 0;
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
draw_frequency(audio_ctrl_t ctrl, audio_stream_t audio_stream,
    fft_config_t fft_config, draw_config_t draw_config)
{
	char keypress;
	int active_bars, draw_start, option, res, title_center;
	u_int i, j;
	float avg, freq, scaled_magnitude;
	u_char data[audio_stream.total_size];
	float pcm[audio_stream.total_samples];
	bar_t bars[draw_config.nbars];
	bin_t bins[fft_config.nbins];

	title_center = draw_config.cols / 2 - 10;

	mvprintw(0, title_center, "Measure Mic Frequency\n");
	refresh();

	nodelay(stdscr, TRUE);
	for (;;) {
		reset_bins(bins, fft_config);
		reset_bars(bars, draw_config, fft_config);

		if ((res = stream(ctrl, audio_stream, data)) != 0) {
			return res;
		}

		if ((res = to_normalized_pcm(audio_stream, data, pcm)) != 0) {
			return res;
		}

		fft(fft_config, bins, pcm);

		/* Attribute a bin to the corresponding bar */
		for (i = 0; i < fft_config.nbins; i++) {
			freq = bins[i].frequency;
			for (j = 0; j < draw_config.nbars; j++) {
				if (freq >= bars[j].fmin &&
				    freq < bars[j].fmax) {
					bars[j].magnitude += bins[i].magnitude;
					bars[j].nbins += 1;
					break;
				}
			}
		}

		active_bars = 0;
		for (i = 0; i < draw_config.nbars; i++) {
			/*
			 * Based on the number of bins / number of bars it is
			 * possible that some bars just have no data. We are
			 * going to skip drawing these so there are no gaps
			 * in the bar graph
			 */
			if (bars[i].nbins <= 0)
				continue;
			active_bars++;
		}

		draw_start = (int)draw_config.x_padding +
			     (int)(draw_config.max_w - active_bars) / 2;
		j = 0;
		for (i = 0; i < draw_config.nbars; i++) {
			if (bars[i].nbins <= 0)
				continue;

			avg = bars[i].magnitude / (float)bars[i].nbins;
			avg = ceilf(avg * FREQ_SCALE_FACTOR);
			scaled_magnitude = fminf(avg,
			    (float)(draw_config.max_h - draw_config.y_padding));
			mvvline(draw_config.y_padding, (int)j + draw_start, ' ',
			    draw_config.max_h);
			mvvline(draw_config.max_h - (int)scaled_magnitude,
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
 * Renders the nav options at the bottom of the screen for the user to see
 */
void
draw_options(void)
{
	int row;
	row = getmaxy(stdscr);
	mvprintw(row - 1, 0, "OPTIONS: ");
	printw("F: FREQ / I: INFO / Q: QUIT");
	refresh();
}
