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
	u_int nsamples, size;
	float samples_needed;
	float samples_per_buffer;
	float buffers_needed;

	samples_needed = ceilf(
	    (float)milliseconds / 1000 * (float)sample_rate * (float)channels);
	bytes_per_sample = precision / STREAM_BYTE_SIZE;
	samples_per_buffer =
	    ceilf((float)buffer_size / (float)bytes_per_sample);
	buffers_needed = ceilf(samples_needed / samples_per_buffer);

	stream->milliseconds = milliseconds;
	stream->channels = channels;
	stream->encoding = encoding;
	stream->total_samples = (u_int)samples_needed;
	stream->precision = precision;
	stream->total_size = 0;

	i = (u_int)samples_needed;
	while (i > 0) {
		/* the size of the buffer is samples_per_buffer or whats left */
		nsamples = (u_int)fminf((float)i, samples_per_buffer);
		size = nsamples * bytes_per_sample;
		stream->total_size += size;
		i = i - nsamples;
	}
	stream->data = malloc(stream->total_size);

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
	u_int i, ns;
	u_char *data;
	ssize_t io_count;
	io_count = 0;

	data = (char *)stream->data;
	for (i = 0; i < stream->total_size; i++) {
		/* the size of the buffer or whats left */
		ns = (u_int)fminf((float)ctrl.config.buffer_size, (float)(stream->total_size - i));
		if (ctrl.mode == CTRL_RECORD) {
			io_count = read(ctrl.fd, data, ns);
		} else {
			io_count = write(ctrl.fd, data, ns);
		}

		i += ns;
		data += ns;
	}

	return 0;
}
