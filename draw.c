#include <curses.h>
#include <stdlib.h>
#include <math.h>

#include "audio_ctrl.h"
#include "audio_rms.h"
#include "audio_stream.h"
#include "draw.h"
#include "error_codes.h"
#include "fft.h"
#include "pcm.h"


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
	} else if (keypress == 'F') {
		return DRAW_FREQ;
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


/*
 * Displays a screen to record audio and display the data in the frequency
 * spectrum.
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
int
draw_frequency(audio_ctrl_t ctrl, audio_stream_t * audio_stream, fft_config_t fft_config)
{
	char keypress;
	int res, option, row, col;
	u_int i, j, start;
	float real, imag;
	void *full_samples;
	float pcm[audio_stream->total_samples];
	bar_t bars[fft_config.bars];
	bin_t bins[fft_config.bins];
	cplx buf[fft_config.size];

	row = getmaxy(stdscr);
	col = getmaxx(stdscr);

	move(0,0);
	full_samples = malloc(audio_stream->total_size);

	for (i = 0; i < fft_config.bins; i++) {
		bins[i].magnitude = 0.0f;
		bins[i].frequency = (float)i * (float) fft_config.fs / (float) fft_config.size;
	}

	for (i = 0; i < fft_config.bars; i++) {
		float frac_start = (float)i / (float)fft_config.bars;
		float frac_end = (float)(i+1) / (float)fft_config.bars;
		bars[i].f_min = fft_config.f_min * powf(fft_config.f_max / fft_config.f_min, frac_start);
		bars[i].f_max = fft_config.f_min * powf(fft_config.f_max / fft_config.f_min, frac_end);
	}

	nodelay(stdscr, TRUE);
	for (;;) {
		for (i = 0; i < fft_config.bins; i++) {
			bins[i].magnitude = 0.0f;
		}
		for (i = 0; i < fft_config.bars; i++) {
			bars[i].magnitude = 0.0f;
			bars[i].bin_count = 0;
		}

		res = stream(ctrl, audio_stream);
		if (res != 0) {
			return res;
		}

		flatten_stream(audio_stream, full_samples);
		if ((res = to_normalized_pcm(full_samples, pcm, audio_stream)) > 0) {
			free(full_samples);
			return res;
		}

		/*
		 * perform the fft for each frame.
		 * place the magnitude in each bin
		 */
		for (i = 0; i < fft_config.frames; i++) {
			start = i * fft_config.size;

			for (j = 0; j < fft_config.size; j++) {
				buf[j] = pcm[start+j];
			}

			fft(buf, fft_config.size);

			for (j = 0; j < fft_config.bins; j++) {
				real = (float)creal(buf[j]);
				imag = (float)cimag(buf[j]);
				bins[j].magnitude += sqrtf(real * real + imag * imag);
			}
		}

		/* Attribute a bin to the corresponding bar */
		for (i = 0; i < fft_config.bins; i++) {
			float freq = bins[i].frequency;
			float avg = bins[i].magnitude / (float) fft_config.frames;
			for (j = 0; j < fft_config.bars; j++) {
				// TODO there is prolly some hashing function that
				// lets me calculate the index instead of trying
				// to iterate over the whole list every time
				if (freq >= bars[j].f_min && freq < bars[j].f_max) {
					bars[j].magnitude += avg;
					bars[j].bin_count += 1;
					break;
				}
			}
		}

		j = 0;
		for (i = 0; i < fft_config.bars; i++) {
			// TODO it would be great to move this to a separate pane
			//mvprintw((int)i, 0, "%f - %f: %f / %d", bars[i].f_min, bars[i].f_max, bars[i].magnitude, bars[i].bin_count);
			/*
			 * Based on the number of bins / number of bars it is
			 * possible that some bars just have no data. We are
			 * going to skip drawing these so there are no gaps
			 * in the bar graph
			 */
			if (bars[i].bin_count <= 0) continue;

			float a = bars[i].magnitude / (float)bars[i].bin_count;
			float scaled_magnitude = fminf(ceilf(a * 1.2f), (float)row);
			mvvline(0, (int)j+60, ' ', row);
			mvvline(row-(int)scaled_magnitude, (int)j + 60, '|', (int)scaled_magnitude);
			j++;
		}
		refresh();

		/* listen for input */
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DRAW_FREQ) {
			free(full_samples);
			return option;
		}
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
	void *full_samples;

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
	full_samples = malloc(audio_stream->total_size);

	for (;;) {
		/* record the audio to the stream */
		res = stream(ctrl, audio_stream);
		if (res != 0) {
			return res;
		}

		/* calculate rms */
		flatten_stream(audio_stream, full_samples);
		rms = calc_rms(full_samples, audio_stream->precision,
		    audio_stream->total_samples);
		percent = calc_rms_percent(rms, audio_stream->precision);

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
			free(full_samples);
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
	printw("R: RECORD / F: FREQ / I: INFO / Q: QUIT");
	refresh();
}
