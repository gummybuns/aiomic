/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Zac Brown.
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
#include <stdio.h>
#include <stdlib.h>

#include "colors.h"
#include "error_codes.h"

int
extract_color(int i, struct color *color)
{
	color_content((short)i, &(color->r), &(color->g), &(color->b));
	return 0;
}

int
init_color_pairs(u_int coffset, u_int n, struct color start, struct color end)
{
	int res;
	u_int i;
	short r, g, b;
	float t;

	for (i = 0; i < n; i++) {
		t = (float)i / (float)(n - 1);
		r = (short)((1 - t) * start.r + t * end.r);
		g = (short)((1 - t) * start.g + t * end.g);
		b = (short)((1 - t) * start.b + t * end.b);

		res = init_color((short)(i + coffset), r, g, b);
		if (res != 0)
			return res;

		/* i + 1 since we cannot override the default pair */
		init_pair((short)(i + 1), (short)(i + coffset), -1);
	}

	return 0;
}
