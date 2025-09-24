#include <math.h>
#include <stdlib.h>
#include <unistd.h>

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

/*
 * Convert all buffers on a stream into a single array
 * TODO there is probably a better way to do this with memcpy or something
 */
void *
flatten_stream(audio_stream_t *a_stream)
{
	u_int i, j, s;
	void *flattened;
	u_char *cdata;
	short *sdata;
	float *fdata;
	audio_buffer_t *buffer;

	flattened = malloc(a_stream->total_size);
	s = 0;
	for (i = 0; i < a_stream->buffer_count; i++) {
		buffer = a_stream->buffers[i];
		for (j = 0; j < buffer->size; j++) {
			switch (a_stream->precision) {
			case 8:
				cdata = (u_char *)buffer->data;
				((u_char *)flattened)[s] = cdata[j];
				break;
			case 16:
				sdata = (short *)buffer->data;
				((short *)flattened)[s] = sdata[j];
				break;
			case 32:
			default:
				fdata = (float *)buffer->data;
				((float *)flattened)[s] = fdata[j];
			}
			s++;
		}
	}

	return flattened;
}
