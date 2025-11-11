#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#define E_UNHANDLED 1000

#define E_CTRL_FILE_OPEN 1001
#define E_CTRL_GETINFO 1002
#define E_CTRL_GETFORMAT 1003
#define E_CTRL_SETINFO 1004

#define E_FFT_CONFIG_TOTAL_SAMPLES 1100
#define E_FFT_CONFIG_NSAMPLES_BY_2 1101

#define E_DRW_CONFIG_NBARS 1201
#define E_DRW_CONFIG_NBOXES 1202
#define E_DRW_CONFIG_NBARS_ZERO 1203
#define E_DRW_CONFIG_NBOXES_ZERO 1204
#define E_DRW_CONFIG_BOX_HEIGHT_ZERO 1205

#define E_NO_COLORS 1303
#define E_CHANGE_COLORS 1304
#define E_INIT_COLOR 1305

#define E_FREQ_UNKNOWN_PRECISION 2500
#define E_FREQ_UNSUPPORTED_ENCODING 2501

#define E_STREAM_IO_ERROR 3000

static inline const char * get_error_msg(int code);

static inline const char *
get_error_msg(int code)
{
	switch (code) {
	case E_CTRL_FILE_OPEN:
		return "Failed to open audio device";
	case E_CTRL_GETINFO:
		return "Failed to call AUDIO_GETINFO";
	case E_CTRL_GETFORMAT:
		return "Failed to call AUDIO_GETFORMAT";
	case E_CTRL_SETINFO:
		return "Failed to call AUDIO_SETINFO";
	case E_FFT_CONFIG_TOTAL_SAMPLES:
		return "FFT nsamples cannot be greater than total samples";
	case E_FFT_CONFIG_NSAMPLES_BY_2:
		return "FFT nsamples must be a power of 2";
	case E_DRW_CONFIG_NBARS:
		return "Draw config has nbars that exceeds drawing space";
	case E_DRW_CONFIG_NBARS_ZERO:
		return "Draw config has nbars set to 0";
	case E_DRW_CONFIG_NBOXES:
		return "Draw config has nboxes that exceeds drawing space";
	case E_DRW_CONFIG_NBOXES_ZERO:
		return "Draw config has nboxes set to 0";
	case E_DRW_CONFIG_BOX_HEIGHT_ZERO:
		return "Draw config has box_height set to 0";
	case E_NO_COLORS:
		return "Terminal does not support colors";
	case E_CHANGE_COLORS:
		return "Terminal does not support changing colors";
	case E_INIT_COLOR:
		return "Failed to set color";
	case E_FREQ_UNKNOWN_PRECISION:
		return "Unsupported precision";
	case E_FREQ_UNSUPPORTED_ENCODING:
		return "Unsupported encoding";
	case E_STREAM_IO_ERROR:
		return "Streaming I/O error";
	case E_UNHANDLED:
	default:
		return "Unhandled Error";
	}
}
#endif
