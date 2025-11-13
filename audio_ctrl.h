#ifndef AUDIO_CTRL_H
#define AUDIO_CTRL_H

#include <sys/audioio.h>

#define CTRL_CFG_PAUSE 1
#define CTRL_CFG_PLAY 0

typedef struct audio_config_t {
	u_int buffer_size; /* size of the audio device buffer in bytes */
	u_int channels;    /* number of channels for the audio device */
	u_int encoding;    /* the encoding of the audio device */
	u_int precision;   /* the number of bits per sample */
	u_int sample_rate; /* number of samples per second */
} audio_config_t;

typedef struct audio_ctrl_t {
	int fd;                /* file descriptor to the audio device */
	u_int mode;            /* record vs play */
	audio_config_t config; /* the configuration of the audio device */
	const char *path;            /* the path to the audio device */
} audio_ctrl_t;

int build_audio_ctrl(audio_ctrl_t *ctrl, const char *path, u_int mode);
int update_audio_ctrl(audio_ctrl_t *ctrl, audio_config_t config);
const char *get_encoding_name(u_int encoding);
const char *get_mode(audio_ctrl_t ctrl);

#endif
