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
#include <err.h>
#include <math.h>
#include <stdlib.h>

#include "audio_ctrl.h"
#include "draw.h"
#include "draw_config.h"
#include "error_codes.h"
#include "fft.h"
#include "pcm.h"
#include "xmalloc.h"

/*
 * Print details about the audio controller
 */
static void
print_ctrl(WINDOW *w, struct audio_ctrl ctrl)
{
	const char *mode, *config_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name(ctrl.config.encoding);

	wprintw(w,
	    "Audio Controller\n"
	    "\tdevice:\t\t%s\n"
	    "\tmode:\t\t%s\n"
	    "\tbuffer_size:\t%d\n"
	    "\tsample_rate:\t%d\n"
	    "\tprecision:\t%d\n"
	    "\tchannels:\t%d\n"
	    "\tencoding:\t%s\n"
	    "\tmilliseconds:\t%d\n"
	    "\tsize:\t\t%d\n"
	    "\tsamples:\t%d\n\n",
	    ctrl.path, mode, ctrl.config.buffer_size, ctrl.config.sample_rate,
	    ctrl.config.precision, ctrl.config.channels, config_encoding,
	    ctrl.stream.ms, ctrl.stream.total_size, ctrl.stream.total_samples);
}

/*
 * Print details about the fft configuration
 */
static void
print_fft_config(WINDOW *w, struct fft_config config)
{
	wprintw(w,
	    "FFT\n"
	    "\tfs:\t\t%d\n"
	    "\tnbins:\t\t%d\n"
	    "\tnframes:\t%d\n"
	    "\tnsamples:\t%d\n"
	    "\ttotal_samples:\t%d\n"
	    "\tfmin:\t\t%.2f\n"
	    "\tfmax:\t\t%.2f\n\n",
	    config.fs, config.nbins, config.nframes, config.nsamples,
	    config.total_samples, config.fmin, config.fmax);
}

/*
 * Print details about the draw configurations
 */
static void
print_draw_config(WINDOW *w, struct draw_config config)
{
	wprintw(w,
	    "DRAW_CONFIG\n"
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
	    config.rows, config.cols, config.max_h, config.max_w,
	    config.y_padding, config.x_padding, config.nbars, config.bar_width,
	    config.bar_space, config.use_boxes, config.nboxes, config.box_space,
	    config.box_height, config.use_color, config.bar_color,
	    config.bar_color2);
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

/*
 * Update the scroll position based on a keypress
 */
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
 * Display information about the audio controlers
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_info(struct audio_ctrl *rctrl, struct audio_ctrl *pctrl, struct fft_config fft_config,
    struct draw_config draw_config)
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
		print_ctrl(dpad, *rctrl);
		if (pctrl != NULL) {
			print_ctrl(dpad, *pctrl);
		}
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
reset_bars(struct bar *bars, struct draw_config draw_config, struct fft_config fft_config)
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
 * Calculate the starting x position of the first bar
 */
inline int
calculate_draw_start(struct draw_config draw_config, struct bar *bars)
{
	int active_bars, i;

	active_bars = 0;
	for (i = 0; i < (int)draw_config.nbars; i++) {
		if (bars[i].nbins <= 0)
			continue;
		active_bars++;
	}

	return (int)draw_config.x_padding +
	       (int)(draw_config.max_w -
		     active_bars * (int)draw_config.bar_width -
		     active_bars * (int)draw_config.bar_space) /
		   2;
}

/*
 * Calculate the draw coordinates of the bar
 *
 * coords - the coordinates to store
 * cfg - the draw configuration of the app
 * bi - the bar index
 * xi - the box index
 * draw_start - the x offset of the first bar that is drawn
 * maxy - the maximum height of a bar
 */
inline void
calculate_coords(struct coords *coords, struct draw_config cfg, int bi, int xi,
    int draw_start, int maxy)
{
	int cols, offset, rows, startx, starty;

	rows = cfg.use_boxes ? (int)cfg.box_height : maxy;
	cols = (int)cfg.bar_width;
	startx =
	    (bi * (int)cfg.bar_width) + draw_start + (bi * (int)cfg.bar_space);

	/* TODO i dont really know why i need this offset when in bar mode */
	offset = cfg.use_boxes ? 0 : 1;
	starty = cfg.max_h - (xi + offset) * rows - xi * (int)cfg.box_space;

	coords->rows = rows;
	coords->cols = cols;
	coords->startx = startx;
	coords->starty = starty;
}

/*
 * Displays a screen to record audio and display the data in the frequency
 * spectrum.
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_frequency(struct audio_ctrl *rctrl, struct audio_ctrl *pctrl,
    struct fft_config fft_config, struct draw_config draw_config)
{
	char keypress;
	int draw_start, option, res, draw_height;
	int bari, boxi;
	u_int i, j, c;
	float freq, scaled_magnitude;
	u_char *data = NULL;
	float *pcm = NULL;
	struct bar *bars = NULL;
	struct bin *bins = NULL;
	WINDOW *fwin, ***bwin = NULL;
	struct coords coords;

	data = xreallocarray(data, rctrl->stream.total_size, sizeof(u_char));
	pcm = xreallocarray(pcm, rctrl->stream.total_samples, sizeof(float));
	bars = xreallocarray(bars, draw_config.nbars, sizeof(struct bar));
	bins = xreallocarray(bins, fft_config.nbins, sizeof(struct bin));
	bwin = xreallocarray(bwin, draw_config.nbars, sizeof(WINDOW **));

	memset(bwin, 0, draw_config.nbars * sizeof(WINDOW **));

	for (i = 0; i < draw_config.nbars; i++) {
		bwin[i] = xreallocarray(bwin[i],
		    draw_config.nboxes, sizeof(WINDOW *));
	}

	nodelay(stdscr, TRUE);

	fwin = newwin(draw_config.rows, draw_config.cols, 0, 0);
	wrefresh(fwin);

	for (i = 0; i < draw_config.nbars; i++) {
		for (j = 0; j < draw_config.nboxes; j++) {
			bwin[i][j] = NULL;
		}
	}

	c = 0;
	for (;;) {
		reset_bins(bins, fft_config);
		reset_bars(bars, draw_config, fft_config);

		if ((res = stream(rctrl, data)) != 0) {
			goto finish;
		}

		if (pctrl != NULL) {
			if ((res = stream(pctrl, data)) != 0) {
				goto finish;
			}
		}

		c++;

		if ((res = to_normalized_pcm(data, pcm, rctrl->config.encoding,
			 rctrl->config.precision, rctrl->stream.total_size)) !=
		    0) {
			goto finish;
		}

		fft(fft_config, bins, pcm);

		/* Attribute a bin to the corresponding bar */
		for (i = 0; i < fft_config.nbins; i++) {
			freq = bins[i].frequency;
			for (j = 0; j < draw_config.nbars; j++) {
				if (freq >= bars[j].fmin &&
				    freq < bars[j].fmax) {
					bars[j].nbins += 1;
					bars[j].magnitude +=
					    (bins[i].magnitude -
						bars[j].magnitude) /
					    (float)bars[j].nbins;
					break;
				}
			}
		}

		draw_start = calculate_draw_start(draw_config, bars);
		j = 0;

		werase(fwin);
		mvwprintw(fwin, 0, 0, "count=%d\n", c);

		bari = 0;
		for (i = 0; i < draw_config.nbars; i++) {
			if (bars[i].nbins <= 0)
				continue;

			scaled_magnitude = fminf(bars[i].magnitude,
			    (float)(draw_config.max_h - draw_config.y_padding));
			/* need at least a height of 2 to draw a box */
			scaled_magnitude =
			    scaled_magnitude < 2 ? 2 : scaled_magnitude;

			draw_height = 0;
			boxi = 0;
			while (draw_height < (int)ceilf(scaled_magnitude)) {
				calculate_coords(&coords, draw_config, bari,
				    boxi, draw_start, (int)scaled_magnitude);
				delwin(bwin[i][boxi]);
				bwin[i][boxi] = subwin(fwin, coords.rows,
				    coords.cols, coords.starty, coords.startx);

				if (draw_config.use_color) {
					/* add one because we never override the
					 * first color */
					int pidx = draw_config.ncolors > 1
						       ? boxi + 1
						       : 1;
					wbkgd(bwin[i][boxi],
					    COLOR_PAIR(pidx) | A_REVERSE);
				} else {
					box(bwin[i][boxi], 0, 0);
				}

				draw_height +=
				    coords.rows + (int)draw_config.box_space;
				boxi++;
			}
			bari++;
		}

		wnoutrefresh(fwin);
		doupdate();

		/* listen for input */
		flushinp();
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_FREQ &&
		    option != DRAW_DEBUG) {
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
	for (i = 0; i < draw_config.nbars; i++) {
		free(bwin[i]);
	}
	free(bwin);
	free(bins);
	free(bars);
	free(pcm);
	free(data);
	delwin(fwin);
	return res;
}
