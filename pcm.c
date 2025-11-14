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
#include "pcm.h"
#include "auconv.h"
#include "error_codes.h"

/*
 * Set the swap function of a converter
 *
 * Swapping is meant to be done when an audio encoding is used that differs from
 * the machine's endian-ness.
 */
static inline int
set_swap_func(pcm_converter_t *converter, u_int precision, u_int encoding)
{
	converter->swap_func = NULL;

	switch (encoding) {
	case AUDIO_ENCODING_ULINEAR:
	case AUDIO_ENCODING_SLINEAR:
		break;
	case AUDIO_ENCODING_SLINEAR_LE:
	case AUDIO_ENCODING_ULINEAR_LE:
		if (BYTE_ORDER == BIG_ENDIAN) {
			if (precision == 8) {
				converter->swap_func = NULL;
			} else if (precision == 16) {
				converter->swap_func = swap_bytes;
			} else if (precision == 32) {
				converter->swap_func = swap_bytes32;
			} else {
				return E_FREQ_UNKNOWN_PRECISION;
			}
		}
		break;
	case AUDIO_ENCODING_SLINEAR_BE:
	case AUDIO_ENCODING_ULINEAR_BE:
		if (BYTE_ORDER == LITTLE_ENDIAN) {
			if (precision == 8) {
				converter->swap_func = NULL;
			} else if (precision == 16) {
				converter->swap_func = swap_bytes;
			} else if (precision == 32) {
				converter->swap_func = swap_bytes32;
			} else {
				return E_FREQ_UNKNOWN_PRECISION;
			}
		}
		break;
	default:
		return E_FREQ_UNSUPPORTED_ENCODING;
	}

	return 0;
}

/*
 * Set the sign function of a converter for an unsigned audio encoding
 */
static inline int
set_sign_func(pcm_converter_t *converter, u_int precision, u_int encoding)
{
	converter->sign_func = NULL;

	switch (encoding) {
	case AUDIO_ENCODING_SLINEAR_LE:
	case AUDIO_ENCODING_SLINEAR_BE:
	case AUDIO_ENCODING_SLINEAR:
		break;
	case AUDIO_ENCODING_ULINEAR:
	case AUDIO_ENCODING_ULINEAR_LE:
	case AUDIO_ENCODING_ULINEAR_BE:
		if (precision == 8) {
			converter->sign_func = change_sign8;
		} else if (precision == 16 && BYTE_ORDER == LITTLE_ENDIAN) {
			converter->sign_func = change_sign16_le;
		} else if (precision == 16 && BYTE_ORDER == BIG_ENDIAN) {
			converter->sign_func = change_sign16_be;
		} else if (precision == 32 && BYTE_ORDER == LITTLE_ENDIAN) {
			converter->sign_func = change_sign32_le;
		} else if (precision == 32 && BYTE_ORDER == BIG_ENDIAN) {
			converter->sign_func = change_sign32_be;
		} else {
			return E_FREQ_UNKNOWN_PRECISION;
		}
		break;
	default:
		return E_FREQ_UNSUPPORTED_ENCODING;
	}

	return 0;
}

/*
 * Set the normalization function for a converter based on the precision
 */
static inline int
set_normalize_func(pcm_converter_t *converter, u_int precision)
{
	switch (precision) {
	case 8:
		converter->normalize_func = normalize8;
		break;
	case 16:
		converter->normalize_func = normalize16;
		break;
	case 32:
		converter->normalize_func = normalize32;
		break;
	default:
		return E_FREQ_UNKNOWN_PRECISION;
	}

	return 0;
}

/*
 * Build an pcm_converter
 */
int
build_converter(pcm_converter_t *converter, u_int precision, u_int encoding)
{
	int err;

	if ((err = set_swap_func(converter, precision, encoding)) > 0) {
		return err;
	}

	if ((err = set_sign_func(converter, precision, encoding)) > 0) {
		return err;
	}

	if ((err = set_normalize_func(converter, precision)) > 0) {
		return err;
	}

	return 0;
}

/*
 * Convert the raw audio data into normalized pcm data
 */
int
to_normalized_pcm(audio_stream_t audio_stream, u_char *data, float *pcm)
{
	u_char *c;
	u_int encoding, i, j, precision;
	u_int inc;
	int err;
	pcm_converter_t converter;

	precision = audio_stream.precision;
	encoding = audio_stream.encoding;

	if ((err = build_converter(&converter, precision, encoding)) > 0) {
		return err;
	}

	inc = precision / STREAM_BYTE_SIZE;
	c = data;
	j = 0;
	for (i = 0; i < audio_stream.total_size; i += inc) {
		if (converter.swap_func != NULL) {
			converter.swap_func(c);
		}
		if (converter.sign_func != NULL) {
			converter.sign_func(c);
		}
		pcm[j] = converter.normalize_func(c);

		j++;
		c += inc;
	}
	return 0;
}
