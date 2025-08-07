#include <curses.h>
#include <stdlib.h>
#include <math.h>

#include "kiss_fft.h"

#include "audio_ctrl.h"
#include "audio_rms.h"
#include "audio_stream.h"
#include "draw.h"
#include "error_codes.h"

/*
 * Print details about the audio controller
 */
static void
print_ctrl(audio_ctrl_t ctrl)
{
	const char *mode, *config_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name(ctrl.config.encoding);

	printw("Audio Controller\n"
	       "\tDevice:\t\t%s\n"
	       "\tMode:\t\t%s\n"
	       "\tConfiguration:\n"
	       "\t\tbuffer_size:\t\t%d\n"
	       "\t\tsample_rate:\t\t%d\n"
	       "\t\tprecision:\t\t%d\n"
	       "\t\tchannels:\t\t%d\n"
	       "\t\tencoding:\t\t%s\n"
	       "\t\tpause:\t\t\t%d\n",
	    ctrl.path, mode, ctrl.config.buffer_size, ctrl.config.sample_rate,
	    ctrl.config.precision, ctrl.config.channels, config_encoding,
	    ctrl.config.pause);
}

/*
 * Check if the user pressed any of the navigation options
 */
static int
check_options(int keypress)
{
	if (keypress == 'I') {
		return DRAW_INFO;
	} else if (keypress == 'R') {
		return DRAW_RECORD;
	} else if (keypress == 'Q') {
		return DRAW_EXIT;
	} else {
		return 0;
	}
}

/*
 * Display information about the audio controlers + streams
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream)
{
	char keypress;
	int option;

	move(0, 0);
	nodelay(stdscr, FALSE);
	print_ctrl(ctrl);
	for (;;) {
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_INFO) {
			return option;
		}
	}
}

inline int
*to_normalized_pcm(void *full_sample, float *pcm, audio_stream_t *audio_stream)
{
	u_int encoding, i, precision, total_samples;
	char *cdata;
	short *sdata;
	float *fdata;

	precision = audio_stream->precision;
	total_samples = audio_stream->total_samples;
	encoding = audio_stream->encoding;

	switch (encoding) {
	/* TODO assuming that the endian-ness matches the cpu */
	case CTRL_SLINEAR_LE:
	case CTRL_SLINEAR_BE:
		if (precision == 8) {
			cdata = (char *)full_sample;
			for (i = 0; i < total_samples; i++) {
				pcm[i] = (float)full_sample[i] / 128.0f;
			}
		} else if (precision == 16) {
			sdata = (short *)full_sample;
			for (i = 0; i < total_samples; i++) {
				pcm[i] = (float)full_sample[i] / 32768.0f;
			}
		} else if (precision == 32) {
			for (i = 0; i < total_samples; i++) {
				pcm[i] = (float)full_sample[i] / 2147483647.0f;
			}
		} else {
			return E_FREQ_UNKNOWN_PRECISION;
		}
	default:
		return E_FREQ_UNSUPPORTED_ENCODING
	return 0;
}


int
draw_frequency(audio_ctrl_t ctrl, audio_stream_t *audio_stream)
{
	char keypress;
	int res;
	void *full_sample;
	float *pcm;
	/*
	 * We have the total_samples (audio_stream.total_samples)
	 * we have the size of the fft (1024 hard coded)
	 * therefore there are total_samples / 1024 fft_frames we are going to compute
	 * every frame has a start_idx / end_idx. that tells us where to look in the buffer
	 */
	u_int bars = 32;
	u_int fft_size = 1024;
	u_int bins = fft_size / 2;
	u_int frames = audio_stream->total_samples / fft_size;
	float magnitudes[frames][bins];
	float avg[bins];
	kiss_fft_cpx in[fft_size], out[fft_size];

	for (;;) {
		// todo - i need to zero out the avg after each loop
		res = stream(ctrl, audio_stream);
		if (res != 0) {
			return res;
		}

		full_samples = flatten_stream(audio_stream);
		pcm = malloc(sizeof(float) * audio_stream->total_samples);
		if (to_normalized_pcm(full_samples, pcm, audio_stream) > 0) {
			free(full_samples);
			free(pcm);
			return res;
		}

		for (int i = 0; i < frames; i++) {
			int start = i * fft_size;

			/* initialize the fft in data */
			for (int j = 0; j < fft_size; j++) {
				in[j].r = pcm[start+j];
				in[j].i = 0.0f;
			}

			/* perform the fft */
			kiss_fft_cfg cfg = kiss_fft_alloc(fft_size, 0, NULL, NULL);
			kiss_fft(cfg, in, out);

			/* get the magitudes for each bin in the current frame */
			/* calculate the running average */
			for (int j = 0; j < bins; j++) {
				magnitudes[i][j] = sqrtf(out[j].r * out[j].r + out[j].i * out[j].i);
			}
		}

		/* calculate the average across all frames */
		for (int i = 0; i < frames; i ++) {
			for (int j = 0; j < bins; j++) {
				avg[j] += magnitudes[i][j];
			}
			// todo - use the rms instead of just the mean
			avg[j] = avg[j] / frames;
		}

		/* calculate the average for each bar */

		/* calculate the height based of each bar */

		/* draw each bar */
		free(pcm);
		free(full_samples);
	}
}


/*
 * Displays a screen to record audio and show the intensity based on the
 * Root Mean Square
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_intensity(audio_ctrl_t ctrl, audio_stream_t *audio_stream)
{
	char keypress;
	int title_center;
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	int option;
	int res;
	float rms, percent;
	void *full_sample;

	getmaxyx(stdscr, row, col);
	y_padding = col / 10;
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;
	title_center = col / 2 - 10;

	mvprintw(0, title_center, "Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0%%");
	mvprintw(3 + x_padding, bar_start + bar_distance / 2 - 2, "50%%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100%%");
	refresh();

	nodelay(stdscr, TRUE);

	for (;;) {
		/* record the audio to the stream */
		res = stream(ctrl, audio_stream);
		if (res != 0) {
			return res;
		}

		/* calculate rms */
		full_sample = flatten_stream(audio_stream);
		rms = calc_rms(full_sample, audio_stream->precision,
		    audio_stream->total_samples);
		percent = calc_rms_percent(rms, audio_stream->precision);
		free(full_sample);

		if (percent < 0) {
			return E_RMS_UNKNOWN_PRECISION;
		}

		/* draw */
		draw_length =
		    (int)((float)bar_distance * (percent / (float)100.0));
		mvhline(2, 0, ' ', col);
		mvprintw(2, title_center + 2, "RMS:\t%d", (int)rms);
		mvhline(2 + x_padding, bar_start, ' ', bar_distance);
		mvhline(2 + x_padding, bar_start, '=', draw_length);
		move(1, 0);
		refresh();

		/* listen for input */
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_RECORD) {
			return option;
		}
	}
}

/*
 * Renders the nav options at the bottom of the screen for the user to see
 */
void
draw_options(void)
{
	int row;
	row = getmaxy(stdscr);
	mvprintw(row - 1, 0, "OPTIONS: ");
	printw("R: RECORD / I: INFO / Q: QUIT");
	refresh();
}
