#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "auconv.h"
#include "audio_ctrl.h"
#include "audio_stream.h"
#include "error_codes.h"

/*
 * Build an audio stream from the audio controller
 */
int
build_stream_from_ctrl(audio_ctrl_t ctrl, u_int ms, audio_stream_t *stream)
{
	return build_stream(ms, ctrl.config.channels, ctrl.config.sample_rate,
	    ctrl.config.buffer_size, ctrl.config.precision,
	    ctrl.config.encoding, stream);
}

/*
 * Build an audio stream
 *
 * As an example:
 * milliseconds = 2000
 * channels = 2
 * sample_rate = 1000. That means I get 1000 samples per second
 * buffer_size = 1000
 * precision = 16
 *
 * I need 2000 samples _per_ channel (sample_rate * milliseconds / 1000).
 * Therefore I need 4000 samples total (samples_needed)
 *
 * each sample is 2 bytes (16 bits - bytes_per_sample = precision / 8),
 *
 * and one buffer can only support 1000 bytes total (buffer_size).
 * that means there are 500 samples per buffer.
 * that means i need 8 buffers (samples_needed / samples_per_buffer)
 */
int
build_stream(u_int milliseconds, u_int channels, u_int sample_rate,
    u_int buffer_size, u_int precision, u_int encoding, audio_stream_t *stream)
{
	u_int i;
	u_int bytes_per_sample;
	u_int nsamples;
	float samples_needed;
	float samples_per_buffer;
	float buffers_needed;
	audio_buffer_t *buffer;

	samples_needed = ceilf(
	    (float)milliseconds / 1000 * (float)sample_rate * (float)channels);
	bytes_per_sample = precision / STREAM_BYTE_SIZE;
	samples_per_buffer =
	    ceilf((float)buffer_size / (float)bytes_per_sample);
	buffers_needed = ceilf(samples_needed / samples_per_buffer);

	stream->milliseconds = milliseconds;
	stream->channels = channels;
	stream->encoding = encoding;
	stream->buffers =
	    malloc((size_t)buffers_needed * sizeof(audio_buffer_t *));
	stream->total_samples = (u_int)samples_needed;
	stream->precision = precision;
	stream->buffer_count = 0;
	stream->total_size = 0;

	i = (u_int)samples_needed;
	while (i > 0) {
		buffer = malloc(sizeof(audio_buffer_t));

		/* the size of the buffer is samples_per_buffer or whats left */
		nsamples = (u_int)fminf((float)i, samples_per_buffer);

		buffer->size = nsamples * bytes_per_sample;
		buffer->precision = precision;
		buffer->samples = nsamples;
		buffer->data = malloc(buffer->size);
		stream->buffers[stream->buffer_count] = buffer;
		stream->buffer_count++;
		stream->total_size += buffer->size;
		i = i - nsamples;
	}

	return 0;
}

/*
 * Record or Play the audio stream based on the audio controller mode
 *
 * TODO: consider validations to ensure the stream + ctrl have the same
 * endoding
 */
int
stream(audio_ctrl_t ctrl, audio_stream_t *stream)
{
	u_int i;
	ssize_t io_count;
	io_count = 0;

	for (i = 0; i < stream->buffer_count; i++) {
		audio_buffer_t *buffer = stream->buffers[i];

		if (ctrl.mode == CTRL_RECORD) {
			io_count = read(ctrl.fd, buffer->data, buffer->size);
		} else {
			io_count = write(ctrl.fd, buffer->data, buffer->size);
		}

		if (io_count < 0) {
			return E_STREAM_IO_ERROR;
		}
	}

	return 0;
}

/*
 * Free up all buffers on the stream
 */
int
clean_buffers(audio_stream_t *stream)
{
	u_int i;

	for (i = 0; i < stream->buffer_count; i++) {
		free(stream->buffers[i]->data);
		free(stream->buffers[i]);
	}
	free(stream->buffers);

	return 0;
}

int
flatten_stream(audio_stream_t *a_stream, void *flattened)
{
    u_int i;
    u_char *dst = (u_char *)flattened;
    audio_buffer_t *buffer;

    for (i = 0; i < a_stream->buffer_count; i++) {
        buffer = a_stream->buffers[i];
        memcpy(dst, buffer->data, buffer->size);
        dst += buffer->size;
    }

    return 0;
}

static inline int
build_converter(converter_t *converter, u_int precision, u_int encoding)
{
	converter->swap_func = NULL;
	converter->sign_func = NULL;
	converter->normalize_func = NULL;

	switch (encoding) {
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

	if (precision == 8) {
		converter->normalize_func = normalize8;
	} else if (precision == 16) {
		converter->normalize_func = normalize16;
	} else if (precision == 32) {
		converter->normalize_func = normalize32;
	} else {
		return E_FREQ_UNKNOWN_PRECISION;
	}

	return 0;
}

int
to_normalized_pcm(void *full_sample, float *pcm, audio_stream_t *audio_stream)
{
	u_int encoding, i, j, precision, total_samples;
	void (*conv_func) (u_char *, int);
	u_char *cdata, *c;
	converter_t converter;
	u_int inc;

	precision = audio_stream->precision;
	total_samples = audio_stream->total_samples;
	encoding = audio_stream->encoding;

	build_converter(&converter, precision, encoding);

	inc = precision / STREAM_BYTE_SIZE;
	cdata = (u_char *)full_sample;
	c = cdata;
	j = 0;
	for (i = 0; i < audio_stream->total_size; i += inc) {
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
