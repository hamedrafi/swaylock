#ifndef _SWAYLOCK_MATRIX_H
#define _SWAYLOCK_MATRIX_H
#include <cairo/cairo.h>
#include <stdint.h>

void render_matrix_background(cairo_t *cairo, int width, int height,
		double time, uint32_t seed);

#endif
