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
#include <sys/audioio.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <math.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "audio_ctrl.h"
#include "error_codes.h"

#define PRINFO(m, i) (m == AUMODE_RECORD ? &(i.record) : &(i.play))

inline u_int
calc_total_samples(audio_ctrl_t *ctrl)
{
	return (u_int) ceilf((float)ctrl->stream.ms / 1000 * (float)ctrl->config.sample_rate * (float)ctrl->config.channels);
}

inline u_int
calc_total_size(audio_ctrl_t *ctrl)
{
	return ctrl->stream.total_samples * ctrl->config.precision / STREAM_BYTE_SIZE;
}

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

char
is_pad_device(const char *path)
{
	int res, cflags;
	size_t nmatch;
	regex_t preg;

	cflags = REG_EXTENDED | REG_ICASE | REG_NOSUB;
	nmatch = 0;
	regcomp(&preg, "/dev/pad[[:digit:]]+", cflags);
	res = regexec(&preg, path, nmatch, 0, 0);
	regfree(&preg);

	return res == 0;
}

/*
 * Initializes an audio controller based on the file path to the audio device
 */
int
build_audio_ctrl(audio_ctrl_t *ctrl, const char *path, u_int mode, u_int ms)
 {
	int fd, oflag;
	audio_info_t info, format;
	struct audio_prinfo *pri, *prf;

	oflag = mode == AUMODE_RECORD ? O_RDONLY : O_WRONLY;
	fd = open(path, oflag);
	if (fd == -1) {
		return E_CTRL_FILE_OPEN;
	}

	ctrl->path = path;
	ctrl->fd = fd;
	ctrl->mode = mode;

	if (is_pad_device(path)) {
		ctrl->config.precision = 16;
		ctrl->config.encoding = AUDIO_ENCODING_SLINEAR_LE;
		ctrl->config.buffer_size = 2000;
		ctrl->config.sample_rate = 44100;
		ctrl->config.channels = 2;
		ctrl->stream.ms = ms;
		ctrl->stream.total_samples = calc_total_samples(ctrl);
		ctrl->stream.total_size = calc_total_size(ctrl);
		return 0;
	}

	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return E_CTRL_GETINFO;
	}
	if (ioctl(ctrl->fd, AUDIO_GETFORMAT, &format) == -1) {
		return E_CTRL_GETFORMAT;
	}

	pri = PRINFO(mode, info);
	prf = PRINFO(mode, format);
	pri->buffer_size = prf->buffer_size;
	pri->sample_rate = prf->sample_rate;
	pri->precision = prf->precision;
	pri->channels = prf->channels;
	pri->encoding = prf->encoding;

	if (ioctl(ctrl->fd, AUDIO_SETINFO, &info) == -1) {
		return E_CTRL_SETINFO;
	}

	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return E_CTRL_GETINFO;
	}


	pri = PRINFO(mode, info);
	ctrl->config.precision = pri->precision;
	ctrl->config.encoding = pri->encoding;
	ctrl->config.buffer_size = pri->buffer_size;
	ctrl->config.sample_rate = pri->sample_rate;
	ctrl->config.channels = pri->channels;

	ctrl->stream.ms = ms;
	ctrl->stream.total_samples = calc_total_samples(ctrl);
	ctrl->stream.total_size = calc_total_size(ctrl);

	return 0;
}

int
update_audio_ctrl(audio_ctrl_t *ctrl, audio_config_t cfg)
{
	audio_info_t info;
	struct audio_prinfo *pri;

	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return E_CTRL_GETINFO;
	}

	pri = PRINFO(ctrl->mode, info);

	if (cfg.buffer_size > 0) pri->buffer_size = cfg.buffer_size;
	if (cfg.channels > 0) pri->channels = cfg.channels;
	if (cfg.encoding > 0) pri->encoding = cfg.encoding;
	if (cfg.precision > 0) pri->precision = cfg.precision;
	if (cfg.sample_rate > 0) pri->sample_rate = cfg.sample_rate;

	if (ioctl(ctrl->fd, AUDIO_SETINFO, &info) == -1) {
		return E_CTRL_SETINFO;
	}

	/* update ctrl to reflect changes */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return E_CTRL_GETINFO;
	}

	pri = PRINFO(ctrl->mode, info);
	ctrl->config.precision = pri->precision;
	ctrl->config.encoding = pri->encoding;
	ctrl->config.buffer_size = pri->buffer_size;
	ctrl->config.sample_rate = pri->sample_rate;
	ctrl->config.channels = pri->channels;

	ctrl->stream.total_samples = calc_total_samples(ctrl);
	ctrl->stream.total_size = calc_total_size(ctrl);

	return 0;
}

/*
 * Record or Play the audio stream based on the audio controller mode
 */
int
stream(audio_ctrl_t *ctrl, u_char *data)
{
	u_int i, ns;
	ssize_t io_count;
	io_count = 0;
	i = 0;

	while (i < ctrl->stream.total_size) {
		/* the size of the buffer or whats left */
		ns = (u_int)fminf((float)ctrl->config.buffer_size,
		    (float)(ctrl->stream.total_size - i));
		if (ctrl->mode == AUMODE_RECORD) {
			io_count = read(ctrl->fd, data, ns);
		} else {
			io_count = write(ctrl->fd, data, ns);
		}

		if (io_count < 0) {
			return E_STREAM_IO_ERROR;
		}

		i += ns;
		data += ns;
	}

	return 0;
}
