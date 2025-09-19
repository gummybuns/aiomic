#ifndef AUDIO_DRAW_H
#define AUDIO_DRAW_H

#include "audio_ctrl.h"
#include "audio_stream.h"

#define DRAW_EXIT -1
#define DRAW_RECORD 1
#define DRAW_INFO 2
#define DRAW_FREQ 3

int draw_info(audio_ctrl_t ctrl, audio_stream_t audio_stream);
int draw_intensity(audio_ctrl_t rctrl, audio_stream_t *audio_stream);
int draw_frequency(audio_ctrl_t ctrl, audio_stream_t *audio_stream);
void draw_options(void);
#endif
