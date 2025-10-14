#include <sys/audioio.h>
#include <sys/ioctl.h>

#include <err.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "draw_config.h"
#include "error_codes.h"
#include "pcm.h"

#define STREAM_DURATION 250

static inline int
draw_rms(audio_ctrl_t ctrl, audio_stream_t *audio_stream,
    draw_config_t draw_config)
{
	int draw_length;
	int res;
	u_int i;
	float rms, percent;
	float pcm[audio_stream->total_samples];

	printf("\n\n");

	for (;;) {
		/* record the audio to the stream */
		if ((res = stream(ctrl, audio_stream)) != 0) {
			return res;
		}

		/* calculate rms */
		if ((res = to_normalized_pcm(audio_stream, pcm)) > 0) {
			return res;
		}

		rms = 0;
		for (i = 0; i < audio_stream->total_samples; i++) {
			rms += pcm[i] * pcm[i];
		}
		rms = sqrtf(rms / (float)audio_stream->total_samples);
		// TODO it should really be *100 but 1000 makes it "look nicer"
		//  if i keep the 1000 i need to ensure it never goes over 100%
		//  prolly make the scale a draw_config option
		percent = rms * 1000;

		/* draw */
		draw_length =
		    (int)((float)draw_config.nbars * (percent / (float)100.0));
		draw_length = draw_length > (int)draw_config.nbars
				  ? (int)draw_config.nbars
				  : draw_length;
		printf("\33[A");
		printf("\33[2K\r %*s", draw_config.x_padding, "");
		// printf("%d", draw_length);
		for (i = 0; i < (u_int)draw_length; i++) {
			printf("=");
		}
		printf("\n%*s0", draw_config.x_padding, "");
		printf("%*s100", draw_config.nbars, "");
		// printf("\n");
		fflush(stdout);
	}
}

static inline int
build_draw_config(draw_config_t *config)
{
	int x_padding, y_padding;
	struct winsize ws;

	if ((ioctl(0, TIOCGWINSZ, &ws)) != 0) {
		return E_UNHANDLED;
	}

	x_padding = (int)((float)ws.ws_col * PADDING_PCT);
	y_padding = (int)((float)5 * PADDING_PCT);

	config->rows = 5;
	config->cols = ws.ws_col;
	config->x_padding = x_padding;
	config->y_padding = y_padding;
	config->max_h = config->rows - y_padding * 2;
	config->max_w = config->cols - x_padding * 2;
	config->nbars = (u_int)config->max_w;

	return 0;
}

int
main(int argc, char *argv[])
{
	int res;
	char *path;
	audio_ctrl_t ctrl;
	audio_stream_t str;
	draw_config_t draw_config;

	setprogname(argv[0]);

	if (argc <= 1) {
		errno = EINVAL;
		err(1, "Specify an audio device");
	}

	path = argv[1];

	if ((res = build_draw_config(&draw_config)) != 0) {
		err(1, "Failed to init window: %d", res);
	}

	if ((res = build_audio_ctrl(&ctrl, path, AUMODE_RECORD)) != 0) {
		err(1, "Failed to build record audio controller: %d", res);
	}

	if ((res = build_stream_from_ctrl(ctrl, STREAM_DURATION, &str)) != 0) {
		err(1, "Failed to build audio stream: %d", res);
	}

	if ((res = draw_rms(ctrl, &str, draw_config)) != 0) {
		err(1, "Failed to meausre mic: %d", res);
	}
	return 0;
}
