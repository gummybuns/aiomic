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

static void
print_stream(audio_stream_t audio_stream)
{
	const char *config_encoding;

	config_encoding = get_encoding_name(audio_stream.encoding);

	printw("Audio Stream\n"
		"\tchannels:\t\t%d\n"
		"\tmilliseconds:\t\t%d\n"
		"\tprecision:\t\t%d\n"
		"\ttotal_size:\t\t%d\n"
		"\ttotal_samples:\t\t%d\n"
		"\tencoding:\t\t%s\n",
		audio_stream.channels, audio_stream.milliseconds, audio_stream.precision, audio_stream.total_size, audio_stream.total_samples, config_encoding);
}

static void
print_fft_config(fft_config_t config)
{
	printw("FFT\n"
		"\tfs:\t\t%d\n"
		"\tnbins:\t\t%d\n"
		"\tnframes:\t\t%d\n"
		"\tnsamples:\t\t%d\n"
		"\ttotal_samples:\t\t%d\n"
		"\tfmin:\t\t%f\n"
		"\tfmax:\t\t%f\n",
		config.fs, config.nbins, config.nframes, config.nsamples, config.total_samples,config.fmin,config.fmax);
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
	} else if (keypress == 'D') {
		return DRAW_DEBUG;
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
draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream, fft_config_t fft_config)
{
	char keypress;
	int option;

	move(0, 0);
	nodelay(stdscr, FALSE);
	print_ctrl(ctrl);
	print_stream(audio_stream);
	print_fft_config(fft_config);
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
	char keypress, debug_on;
	int active_bars, draw_start, option, res, scroll_pos, draw_height, k;
	u_int i, j;
	float avg, freq, scaled_magnitude;
	u_char data[audio_stream.total_size];
	float pcm[audio_stream.total_samples];
	bar_t bars[draw_config.nbars];
	bin_t bins[fft_config.nbins];
	WINDOW *dpad, *fwin;
	WINDOW *bwin[draw_config.nbars][draw_config.nboxes];

	debug_on = 0;
	dpad = NULL;
	scroll_pos = 0;

	nodelay(stdscr, TRUE);

	fwin = newwin(draw_config.rows, draw_config.cols, 0, 0);
	wrefresh(fwin);

	for (i = 0; i < draw_config.nbars; i++) {
		for (j = 0; j < draw_config.nboxes; j++) {
			bwin[i][j] = NULL;
		}
	}

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
			     (int)(draw_config.max_w - active_bars * (int)draw_config.bar_width - active_bars * (int)draw_config.bar_space) / 2;
		j = 0;

		werase(fwin);
		for (i = 0; i < draw_config.nbars; i++) {
			if (bars[i].nbins <= 0)
				continue;

			avg = bars[i].magnitude / (float)bars[i].nbins;
			avg = ceilf(avg * FREQ_SCALE_FACTOR);
			scaled_magnitude = fminf(avg,
			    (float)(draw_config.max_h - draw_config.y_padding));
			// need at least a height of 2 to draw a box
			scaled_magnitude = scaled_magnitude < 2 ? 2 : scaled_magnitude;

			// TODO - i need to delwin and clean up everything each iteration
			k = 0;
			if (draw_config.nboxes == 1) {
				delwin(bwin[i][0]);
				bwin[i][0] = subwin(fwin,(int) scaled_magnitude, (int)draw_config.bar_width, draw_config.max_h - (int)scaled_magnitude, (int)(j * draw_config.bar_width) + draw_start + (int)(j * draw_config.bar_space));
			} else {
				draw_height = 0;
				while (draw_height < scaled_magnitude) {
					delwin(bwin[i][k]);
					bwin[i][k] = subwin(fwin, (int)draw_config.box_height, (int)draw_config.bar_width, draw_config.max_h - (int)k*draw_config.box_height - (int)k*draw_config.box_space, (int)(j * draw_config.bar_width) + draw_start + (int)(j * draw_config.bar_space));
					draw_height += (draw_config.box_height + draw_config.box_space);
					k++;
				}
				k--;
			}

			if (draw_config.use_color) {
				//wbkgd(bars[i].win, COLOR_PAIR(1) | A_REVERSE);
				do {
					// TODO i dont know why i need pidx + 1 but i do
					int pidx = draw_config.ncolors > 1 ? k + 1 : 1;
					wbkgd(bwin[i][k], COLOR_PAIR(pidx) | A_REVERSE);
					k--;
				} while (k >= 0);
			} else {
				do {
					box(bwin[i][k], 0, 0);
					k--;
				} while (k >= 0);
			}
			//wnoutrefresh(bars[i].shadow);
			//wnoutrefresh(bars[i].win);
			j++;
		}
		//wrefresh(fwin);
		wnoutrefresh(fwin);

		/* i have to create a data structure to define the debug
		 * window config. its getting too much at this point...
		 */
		if (debug_on == 1) {
			i = 0;
			avg = bars[i].magnitude / (float)bars[i].nbins;
			wprintw(dpad, " Bar %d (%.2f - %.2f):\n", i, bars[i].fmin, bars[i].fmax);
			for (j = 0; j < fft_config.nbins; j++) {
				if (bins[j].frequency < fft_config.fmin) continue;
				while(bins[j].frequency > bars[i].fmax) {
					i++;
					avg = bars[i].magnitude / (float)bars[i].nbins;
					wprintw(dpad, " Bar %d (%.2f - %.2f):\n", i, bars[i].fmin, bars[i].fmax);
				}
				wprintw(dpad,"\tBin %d (%.2f): %.2f\n", j, bins[j].frequency, bins[j].magnitude);
			}
			wscrl(dpad, scroll_pos);
			wmove(dpad, 10, 100);
			box(dpad, 0, 0);
			//wnoutrefresh(dpad);
			pnoutrefresh(dpad, 0, 0, 0, draw_config.cols / 2 -1, 10, draw_config.cols);
		}
		doupdate();

		/* listen for input */
		flushinp();
		keypress = (char)getch();
		if (keypress == 'j') {
			scroll_pos++;
		}
		if (keypress == 'k') {
			scroll_pos--;
		}
		if (keypress == 'D' && debug_on == 0) {
			debug_on = 1;
			dpad = newpad(draw_config.nbars + fft_config.nbins - 1, draw_config.cols / 2);
			scrollok(dpad, TRUE);
		} else if (keypress == 'D' && debug_on == 1) {
			debug_on = 0;
			delwin(dpad);
		}
		option = check_options(keypress);
		if (option != 0 && option != DRAW_FREQ && option != DRAW_DEBUG) {
			// TODO free bar windows
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
	mvprintw(row - 2, 1, "OPTIONS: ");
	printw("F: FREQ / I: INFO / Q: QUIT");
	refresh();
}
