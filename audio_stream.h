#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "audio_ctrl.h"

#define STREAM_BYTE_SIZE 8

typedef struct audio_buffer_t {
	u_int samples;   /* number of samples in the buffer */
	u_int size;      /* total memory size of buffer */
	u_int precision; /* number of bits per sample */
	void *data;      /* the buffer of data */
} audio_buffer_t;

typedef struct audios_stream_t {
	u_int buffer_count;  /* number of buffers needed */
	u_int channels;      /* number of channels on the audio device */
	u_int milliseconds;  /* duration of the stream */
	u_int precision;     /* the number of bits per sample */
	u_int total_size;    /* total memory size of all buffers */
	u_int total_samples; /* total number of samples across all buffers */
	u_int encoding;      /* the encoding of the audio device */
	audio_buffer_t **buffers; /* the buffers that hold the data */
} audio_stream_t;

int build_stream(u_int milliseconds, u_int channels, u_int sample_rate,
    u_int buffer_size, u_int precision, u_int encoding, audio_stream_t *stream);

int build_stream_from_ctrl(audio_ctrl_t ctrl, u_int ms, audio_stream_t *stream);
int stream(audio_ctrl_t ctrl, audio_stream_t *stream);
int clean_buffers(audio_stream_t *stream);
int to_normalized_pcm(void *full_sample, float *pcm, audio_stream_t *audio_stream);
int flatten_stream(audio_stream_t *stream, void *flattened);
#endif
