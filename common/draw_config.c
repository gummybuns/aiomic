#include <sys/audioio.h>

#include "draw_config.h"
#include "error_codes.h"

int
validate_draw_config(draw_config_t *config)
{
	u_int computed;

	if (config->box_height <= 0) {
		return E_DRW_CONFIG_BOX_HEIGHT_ZERO;
	}

	if (config->nbars <= 0) {
		return E_DRW_CONFIG_NBARS_ZERO;
	}

	if (config->nboxes <= 0) {
		return E_DRW_CONFIG_NBOXES_ZERO;
	}

	computed = config->box_height * config->nboxes + config->box_space * config->nboxes;
	if (computed > config->max_h) {
		return E_DRW_CONFIG_NBOXES;
	}

	computed = config->nbars * config->bar_width + config->nbars * config->bar_space;
	if (computed > (u_int)config->max_w) {
		return E_DRW_CONFIG_NBARS;
	}

	return 0;
}
