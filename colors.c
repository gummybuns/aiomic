#include <curses.h>
#include <stdlib.h>
#include <stdio.h>

#include "colors.h"
#include "error_codes.h"

int
extract_color(int i, color_t *color)
{
	color_content(i, &(color->r), &(color->g), &(color->b));
	return 0;
}

int
init_color_pairs(int coffset, int n, color_t start, color_t end)
{
	int i, res;
	short r, g, b;
	float t;

	for (i = 0; i < n; i++) {
		t = (float)i / (float)(n - 1);
		r = (int)((1-t) * start.r + t * end.r);
		g = (int)((1-t) * start.g + t * end.g);
		b = (int)((1-t) * start.b + t * end.b);

		res = init_color(i + coffset, r, g, b);
		if (res != 0) return res;

		/* i + 1 since we cannot override the default pair */
		init_pair(i + 1, i + coffset, -1);
	}

	return 0;
}
