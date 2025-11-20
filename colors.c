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
init_color_pairs(color_pair_t **pairs, int n, color_t start, color_t end)
{
	int i, res;
	float t;
	color_pair_t *cp;

	for (i = 0; i < n; i++) {
		cp = pairs[i];
		color_content(i, &(cp->orig.r), &(cp->orig.g), &(cp->orig.b));
		t = (float)i / (float)(n - 1);
		cp->cur.r = (int)((1-t) * start.r + t * end.r);
		cp->cur.g = (int)((1-t) * start.g + t * end.g);
		cp->cur.b = (int)((1-t) * start.b + t * end.b);

		res = init_color(i, cp->cur.r, cp->cur.g, cp->cur.b);
		if (res != 0) return res;
		init_pair(i, i, -1);
	}

	return 0;
}

int
cleanup_colors(color_pair_t **pairs, int n)
{
	int i, res;
	color_pair_t *cp;

	for (i = 0; i < n; i++) {
		cp = pairs[i];
		init_color(i, cp->orig.r, cp->orig.g, cp->orig.b);
		init_pair(i, i, -1);
	}

	free(pairs);
	return 0;
}
