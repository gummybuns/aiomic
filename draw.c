/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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
print_ctrl(WINDOW *w, audio_ctrl_t ctrl)
{
	const char *mode, *config_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name(ctrl.config.encoding);

	wprintw(w, "Audio Controller\n"
	       "\tdevice:\t\t%s\n"
	       "\tmode:\t\t%s\n"
	       "\tbuffer_size:\t%d\n"
	       "\tsample_rate:\t%d\n"
	       "\tprecision:\t%d\n"
	       "\tchannels:\t%d\n"
	       "\tencoding:\t%s\n\n",
	    ctrl.path, mode, ctrl.config.buffer_size, ctrl.config.sample_rate,
	    ctrl.config.precision, ctrl.config.channels, config_encoding);
}

static void
print_stream(WINDOW *w, audio_stream_t audio_stream)
{
	const char *config_encoding;

	config_encoding = get_encoding_name(audio_stream.encoding);

	wprintw(w, "Audio Stream\n"
		"\tchannels:\t%d\n"
		"\tmilliseconds:\t%d\n"
		"\tprecision:\t%d\n"
		"\ttotal_size:\t%d\n"
		"\ttotal_samples:\t%d\n"
		"\tencoding:\t%s\n\n",
		audio_stream.channels, audio_stream.milliseconds, audio_stream.precision, audio_stream.total_size, audio_stream.total_samples, config_encoding);
}

static void
print_fft_config(WINDOW *w, fft_config_t config)
{
	wprintw(w, "FFT\n"
		"\tfs:\t\t%d\n"
		"\tnbins:\t\t%d\n"
		"\tnframes:\t%d\n"
		"\tnsamples:\t%d\n"
		"\ttotal_samples:\t%d\n"
		"\tfmin:\t\t%.2f\n"
		"\tfmax:\t\t%.2f\n\n",
		config.fs, config.nbins, config.nframes, config.nsamples, config.total_samples,config.fmin,config.fmax);
}

static void
print_draw_config(WINDOW *w,draw_config_t config)
{
	wprintw(w,"DRAW_CONFIG\n"
		"\trows:\t\t%d\n"
		"\tcols:\t\t%d\n"
		"\tmax_h:\t\t%d\n"
		"\tmax_w:\t\t%d\n"
		"\ty_padding:\t%d\n"
		"\tx_padding:\t%d\n"
		"\tnbars:\t\t%d\n"
		"\tbar_width:\t%d\n"
		"\tbar_space:\t%d\n"
		"\tuse_boxes:\t%d\n"
		"\tnboxes:\t\t%d\n"
		"\tbox_space:\t%d\n"
		"\tbox_height:\t%d\n"
		"\tuse_color:\t%d\n"
		"\tbar_color:\t%d\n"
		"\tbar_color2:\t%d\n",
		config.rows, config.cols, config.max_h, config.max_w, config.y_padding, config.x_padding, config.nbars, config.bar_width, config.bar_space, config.use_boxes, config.nboxes, config.box_space, config.box_height, config.use_color, config.bar_color, config.bar_color2);
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
	} else if (keypress == 'V') {
		return DRAW_FREQ;
	} else {
		return 0;
	}
}

static void
handle_scroll(char keypress, int *scroll_pos)
{
	if (keypress == 'j') {
		(*scroll_pos)++;
	}
	if (keypress == 'k' && *scroll_pos > 0) {
		(*scroll_pos)--;
	}
}


/*
 * Display information about the audio controlers + streams
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream, fft_config_t fft_config, draw_config_t draw_config)
{
	char keypress;
	int option, scroll_pos;
	WINDOW *dpad;

	scroll_pos = 0;
	dpad = newpad(150, draw_config.cols);
	scrollok(dpad, TRUE);

	move(0, 0);
	nodelay(stdscr, FALSE);
	for (;;) {
		wmove(dpad, 0, 0);
		print_ctrl(dpad, ctrl);
		print_stream(dpad, audio_stream);
		print_fft_config(dpad, fft_config);
		print_draw_config(dpad, draw_config);
		wscrl(dpad, scroll_pos);
		prefresh(dpad, 0, 0, 0, 0, draw_config.rows, draw_config.cols);

		flushinp();
		keypress = (char)wgetch(dpad);
		handle_scroll(keypress, &scroll_pos);
		option = check_options(keypress);
		if (option != 0 && option != DRAW_INFO) {
			delwin(dpad);
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
			goto finish;
		}

		if ((res = to_normalized_pcm(audio_stream, data, pcm)) != 0) {
			goto finish;
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

			k = 0;
			if (draw_config.nboxes == 1) {
				delwin(bwin[i][0]);
				bwin[i][0] = subwin(fwin,(int) scaled_magnitude, (int)draw_config.bar_width, draw_config.max_h - (int)scaled_magnitude, (int)(j * draw_config.bar_width) + draw_start + (int)(j * draw_config.bar_space));
			} else {
				draw_height = 0;
				while (draw_height < (int) ceilf(scaled_magnitude)) {
					delwin(bwin[i][k]);
					bwin[i][k] = subwin(fwin, (int)draw_config.box_height, (int)draw_config.bar_width, draw_config.max_h - (int)k*draw_config.box_height - (int)k*draw_config.box_space, (int)(j * draw_config.bar_width) + draw_start + (int)(j * draw_config.bar_space));
					draw_height += (draw_config.box_height + draw_config.box_space);
					k++;
				}
				k--;
			}

			if (draw_config.use_color) {
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
			j++;
		}
		wnoutrefresh(fwin);
		doupdate();

		/* listen for input */
		flushinp();
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_FREQ && option != DRAW_DEBUG) {
			// TODO free bar windows
			res = option;
			goto finish;
		}
	}
finish:
	for (i = 0; i < draw_config.nbars; i++) {
		for (j = 0; j < draw_config.nboxes; j++) {
			delwin(bwin[i][j]);
		}
	}
	delwin(fwin);
	return res;
}
