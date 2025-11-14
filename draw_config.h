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
#ifndef AUDIO_DRAW_CONFIG_H
#define AUDIO_DRAW_CONFIG_H

#define PADDING_PCT 0.1f

typedef struct draw_config_t {
	char use_color; /* whether to use colors */
	char use_boxes; /* whether to render a bar of boxes */
	int rows;      /* number of rows on screen */
	int cols;      /* number of cols on screen */
	int max_h;     /* max height (including padding) */
	int max_w;     /* max width (including padding) */
	int y_padding; /* padding top/bottom */
	int x_padding; /* padding left/right */
	u_int bar_width; /* width of each bar */
	u_int bar_space; /* amount of space between each bar */
	u_int nbars;   /* number of bars to draw in frequency screen */
	u_int nboxes; /* number of boxes per bar */
	u_int box_space; /* amount of space between each box */
	u_int box_height; /* height of each box */
	u_int ncolors;	/* total number of colors */
	short bar_color; /* color to paint inside of each bar */
	short bar_color2; /* second color to transition to */
} draw_config_t;

int validate_draw_config(draw_config_t *config);

#endif
