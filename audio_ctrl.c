#include <sys/audioio.h>
#include <sys/ioctl.h>

#include <fcntl.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "error_codes.h"

/*
 * Translate standard encoding definitions
 */
const char *
get_encoding_name(u_int encoding)
{
	switch (encoding) {
	case AUDIO_ENCODING_NONE:
		return "NONE";
	case AUDIO_ENCODING_ULAW:
		return "MULAW";
	case AUDIO_ENCODING_ALAW:
		return "ALAW";
	case AUDIO_ENCODING_LINEAR:
		return "LINEAR";
	case AUDIO_ENCODING_LINEAR8:
		return "LINEAR8";
	case AUDIO_ENCODING_SLINEAR:
		return "SLINEAR";
	case AUDIO_ENCODING_SLINEAR_LE:
		return "SLINEAR_LE";
	case AUDIO_ENCODING_SLINEAR_BE:
		return "SLINEAR_BE";
	case AUDIO_ENCODING_ULINEAR:
		return "ULINEAR";
	case AUDIO_ENCODING_ULINEAR_LE:
		return "ULINEAR_LE";
	case AUDIO_ENCODING_ULINEAR_BE:
		return "ULINEAR_BE";
	case AUDIO_ENCODING_MPEG_L1_STREAM:
		return "MPEG_L1_STREAM";
	case AUDIO_ENCODING_MPEG_L1_PACKETS:
		return "MPEG_L1_PACKETS";
	case AUDIO_ENCODING_MPEG_L1_SYSTEM:
		return "MPEG_L1_SYSTEM";
	case AUDIO_ENCODING_MPEG_L2_STREAM:
		return "MPEG_L2_STREAM";
	case AUDIO_ENCODING_MPEG_L2_PACKETS:
		return "MPEG_L2_PACKETS";
	case AUDIO_ENCODING_MPEG_L2_SYSTEM:
		return "MPEG_L2_SYSTEM";
	case AUDIO_ENCODING_AC3:
		return "DOLBY_DIGITAL_AC3";
	default:
		return NULL;
	}
}

/*
 * Get the controller mode as a string
 */
const char *
get_mode(audio_ctrl_t ctrl)
{
	switch (ctrl.mode) {
	case AUMODE_PLAY:
		return "PLAY";
	case AUMODE_RECORD:
		return "RECORD";
	default:
		return NULL;
	}
}

/*
 * Initializes an audio controller based on the file path to the audio device
 */
int
build_audio_ctrl(audio_ctrl_t *ctrl, char *path, u_int mode)
{
	int fd;
	audio_info_t info, format;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return E_CTRL_FILE_OPEN;
	}

	ctrl->path = path;
	ctrl->fd = fd;
	ctrl->mode = mode;

	/* initialize defaults */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return E_CTRL_GETINFO;
	}
	if (ioctl(ctrl->fd, AUDIO_GETFORMAT, &format) == -1) {
		return E_CTRL_GETFORMAT;
	}

	/* set device to use hardware's current settings */
	info.record.pause = format.record.pause;
	info.record.buffer_size = format.record.buffer_size;
	info.record.sample_rate = format.record.sample_rate;
	info.record.precision = format.record.precision;
	info.record.channels = format.record.channels;
	info.record.encoding = format.record.encoding;

	if (ioctl(ctrl->fd, AUDIO_SETINFO, &info) == -1) {
		return E_CTRL_SETINFO;
	}

	/* update ctrl to reflect changes */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return E_CTRL_GETINFO;
	}
	ctrl->config.precision = info.record.precision;
	ctrl->config.encoding = info.record.encoding;
	ctrl->config.buffer_size = info.record.buffer_size;
	ctrl->config.sample_rate = info.record.sample_rate;
	ctrl->config.channels = info.record.channels;

	return 0;
}
