#include "pcm.h"
#include "auconv.h"
#include "error_codes.h"
#include <curses.h>

static inline int
set_swap_func(pcm_converter_t *converter, u_int precision, u_int encoding)
{
	converter->swap_func = NULL;

	switch (encoding) {
	case CTRL_ULINEAR:
	case CTRL_SLINEAR:
		break;
	case CTRL_SLINEAR_LE:
	case CTRL_ULINEAR_LE:
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
	case CTRL_SLINEAR_BE:
	case CTRL_ULINEAR_BE:
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

static inline int
set_sign_func(pcm_converter_t *converter, u_int precision, u_int encoding)
{
	converter->sign_func = NULL;

	switch (encoding) {
	case CTRL_SLINEAR_LE:
	case CTRL_ULINEAR_LE:
	case CTRL_SLINEAR_BE:
	case CTRL_ULINEAR_BE:
		break;
	case CTRL_ULINEAR:
	case CTRL_SLINEAR:
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

int
to_normalized_pcm(audio_stream_t *audio_stream, float *pcm)
{
	u_int encoding, i, j, precision;
	u_char *c;
	u_int inc;
	int err;
	pcm_converter_t converter;

	precision = audio_stream->precision;
	encoding = audio_stream->encoding;

	if ((err = build_converter(&converter, precision, encoding)) > 0) {
		return err;
	}

	inc = precision / STREAM_BYTE_SIZE;
	c = (u_char *)audio_stream->data;
	j = 0;
	for (i = 0; i < audio_stream->total_size; i += inc) {
		if (converter.swap_func != NULL) {
			converter.swap_func(c);
		}
		if (converter.sign_func != NULL) {
			converter.sign_func(c);
		}
		pcm[j] = converter.normalize_func(c);
		refresh();

		j++;
		c += inc;
	}
	return 0;
}
