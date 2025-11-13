/*	$NetBSD: decode.c,v 1.1.8.1 2024/03/12 12:47:40 martin Exp $	*/

/*
 * Copyright (c) 1999 Matthew R. Green
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#ifndef lint
__RCSID("$NetBSD: decode.c,v 1.1.8.1 2024/03/12 12:47:40 martin Exp $");
#endif

#include <sys/audioio.h>
#include <sys/types.h>
#include <sys/time.h>

#include <curses.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <err.h>

void
decode_int(const char *arg, int *intp)
{
	char	*ep;
	int	ret;

	ret = (int)strtoul(arg, &ep, 10);

	if (ep[0] == '\0') {
		*intp = ret;
		return;
	}
	errx(1, "argument `%s' not a valid integer", arg);
}

void
decode_uint(const char *arg, unsigned *intp)
{
	char	*ep;
	unsigned	ret;

	ret = (unsigned)strtoul(arg, &ep, 10);

	if (ep[0] == '\0') {
		*intp = ret;
		return;
	}
	errx(1, "argument `%s' not a valid integer", arg);
}

void
decode_color(const char *arg, short *c)
{
	if (strcasecmp(arg, "black") == 0) {
		*c = COLOR_BLACK;
	} else if (strcasecmp(arg, "red") == 0) {
		*c = COLOR_RED;
	} else if (strcasecmp(arg, "green") == 0) {
		*c = COLOR_GREEN;
	} else if (strcasecmp(arg, "yellow") == 0) {
		*c = COLOR_YELLOW;
	} else if (strcasecmp(arg, "blue") == 0) {
		*c = COLOR_BLUE;
	} else if (strcasecmp(arg, "magenta") == 0) {
		*c = COLOR_MAGENTA;
	} else if (strcasecmp(arg, "cyan") == 0) {
		*c = COLOR_CYAN;
	} else if (strcasecmp(arg, "white") == 0) {
		*c = COLOR_WHITE;
	} else {
		errx(1, "%s is not a valid color", arg);
	}
}

void
decode_encoding(const char *arg, unsigned *u)
{
	if (strcasecmp(arg, "slinear") == 0) {
		*u = AUDIO_ENCODING_SLINEAR;
	} else if (strcasecmp(arg, "ulinear") == 0) {
		*u = AUDIO_ENCODING_ULINEAR;
	} else if (strcasecmp(arg, "slinear_le") == 0) {
		*u = AUDIO_ENCODING_SLINEAR_LE;
	} else if (strcasecmp(arg, "slinear_be") == 0) {
		*u = AUDIO_ENCODING_SLINEAR_BE;
	} else if (strcasecmp(arg, "ulinear_le") == 0) {
		*u = AUDIO_ENCODING_ULINEAR_LE;
	} else if (strcasecmp(arg, "ulinear_be") == 0) {
		*u = AUDIO_ENCODING_ULINEAR_BE;
	} else {
		errx(1, "%s is not a valid encoding", arg);
	}
}
