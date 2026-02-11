#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

#include "audio_ctrl.h"
#include "draw_config.h"
#include "fft.h"

struct thread_context {
	struct audio_ctrl *rctrl;
	struct audio_ctrl *pctrl;
	struct draw_config *draw_config;
	struct fft_config *fft_config;
	u_char *audio_buf;
	pthread_mutex_t *lock;
	sem_t *recording;
	sem_t *rendering;
};
#endif
