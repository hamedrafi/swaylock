#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "matrix.h"

static uint32_t hash(uint32_t value) {
	value ^= value >> 16;
	value *= 0x7feb352d;
	value ^= value >> 15;
	value *= 0x846ca68b;
	return value ^ (value >> 16);
}

void render_matrix_background(cairo_t *cairo, int width, int height,
		double time, uint32_t seed) {
	static const char characters[] =
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ<>[]{}=+-*/:;";
	const int cell_width = 16, cell_height = 22;

	cairo_save(cairo);
	cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgb(cairo, 0, 0, 0);
	cairo_paint(cairo);
	cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);
	cairo_set_antialias(cairo, CAIRO_ANTIALIAS_GRAY);
	cairo_select_font_face(cairo, "monospace",
		CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cairo, 16);

	// Resolve the alphabet once per frame; Cairo caches the rendered glyphs.
	cairo_glyph_t *alphabet = NULL;
	int alphabet_size = 0;
	cairo_status_t status = cairo_scaled_font_text_to_glyphs(
		cairo_get_scaled_font(cairo), 0, 0, characters, -1,
		&alphabet, &alphabet_size, NULL, NULL, NULL);
	if (status != CAIRO_STATUS_SUCCESS ||
			alphabet_size != (int)sizeof(characters) - 1) {
		cairo_glyph_free(alphabet);
		cairo_restore(cairo);
		return;
	}

	uint32_t tick = (uint32_t)(time * 8);
	for (int column = 0; column * cell_width < width; ++column) {
		uint32_t stream = hash(seed + (uint32_t)column);
		int length = 12 + stream % 25;
		double speed = 90 + hash(stream) % 150;
		double cycle = height + (length + 12) * cell_height;
		double head = fmod(time * speed + hash(stream + 1) % (uint32_t)cycle,
			cycle);
		int head_row = (int)(head / cell_height);
		for (int tail = 0; tail < length; ++tail) {
			int row = head_row - tail;
			double y = head - tail * cell_height;
			if (y < 0 || y > height + cell_height) {
				continue;
			}

			uint32_t symbol = hash(stream + (uint32_t)row * 31 + tick);
			cairo_glyph_t glyph = {
				.index = alphabet[symbol % alphabet_size].index,
				.x = column * cell_width + 2,
				.y = y,
			};
			if (tail == 0) {
				cairo_set_source_rgb(cairo, 0.75, 1.0, 0.85);
			} else {
				double brightness = 1.0 - (double)tail / length;
				cairo_set_source_rgb(cairo, 0.02 * brightness,
					0.8 * brightness, 0.18 * brightness);
			}
			cairo_show_glyphs(cairo, &glyph, 1);
		}
	}
	cairo_glyph_free(alphabet);
	cairo_restore(cairo);
}
