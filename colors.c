#include <curses.h>
#include <stdio.h>
#include <stdlib.h>

#include "colors.h"
#include "error_codes.h"

int
extract_color(int i, color_t *color)
{
	color_content((short)i, &(color->r), &(color->g), &(color->b));
	return 0;
}

int
init_color_pairs(u_int coffset, u_int n, color_t start, color_t end)
{
	int res;
	u_int i;
	short r, g, b;
	float t;

	for (i = 0; i < n; i++) {
		t = (float)i / (float)(n - 1);
		r = (short)((1 - t) * start.r + t * end.r);
		g = (short)((1 - t) * start.g + t * end.g);
		b = (short)((1 - t) * start.b + t * end.b);

		res = init_color((short)(i + coffset), r, g, b);
		if (res != 0)
			return res;

		/* i + 1 since we cannot override the default pair */
		init_pair((short)(i + 1), (short)(i + coffset), -1);
	}

	return 0;
}
