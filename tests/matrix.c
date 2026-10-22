#include <cairo/cairo.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

static void check(bool condition, const char *message) {
	if (!condition) {
		fprintf(stderr, "%s\n", message);
		exit(EXIT_FAILURE);
	}
}

static uint64_t frame_checksum(cairo_t *cairo, double time, uint32_t seed) {
	cairo_surface_t *surface = cairo_get_target(cairo);
	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	cairo_matrix_t matrix;
	cairo_get_matrix(cairo, &matrix);
	render_matrix_background(cairo, width / matrix.xx, height / matrix.yy,
		time, seed);
	check(cairo_status(cairo) == CAIRO_STATUS_SUCCESS, "Cairo rendering failed");
	check(cairo_get_operator(cairo) == CAIRO_OPERATOR_OVER,
		"Rendering changed the caller's compositing operator");
	cairo_matrix_t after;
	cairo_get_matrix(cairo, &after);
	check(after.xx == matrix.xx && after.yy == matrix.yy,
		"Rendering changed the caller's scale");
	cairo_surface_flush(surface);
	unsigned char *data = cairo_image_surface_get_data(surface);
	int stride = cairo_image_surface_get_stride(surface);
	uint64_t checksum = 0;
	size_t green_pixels = 0;
	for (int y = 0; y < height; ++y) {
		uint32_t *row = (uint32_t *)(data + y * stride);
		for (int x = 0; x < width; ++x) {
			uint32_t pixel = row[x];
			check((pixel >> 24) == 0xff, "Background must be fully opaque");
			unsigned red = (pixel >> 16) & 0xff;
			unsigned green = (pixel >> 8) & 0xff;
			unsigned blue = pixel & 0xff;
			if (green > red && green > blue) {
				++green_pixels;
			}
			checksum = checksum * 31 + pixel;
		}
	}
	check(green_pixels > 0, "Background has no green characters");
	return checksum;
}

static void check_viewport(int width, int height, int scale) {
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		width * scale, height * scale);
	cairo_t *cairo = cairo_create(surface);
	cairo_scale(cairo, scale, scale);
	uint64_t first = frame_checksum(cairo, 2.0, 17);
	check(first == frame_checksum(cairo, 2.0, 17),
		"Rendering the same instant must produce the same frame");
	check(first != frame_checksum(cairo, 2.1, 17),
		"The background does not animate as time advances");
	check(first != frame_checksum(cairo, 2.0, 18),
		"Different outputs must have distinct character streams");
	cairo_destroy(cairo);
	cairo_surface_destroy(surface);
}

int main(void) {
	check_viewport(1920, 1080, 1);
	check_viewport(480, 800, 1);
	check_viewport(640, 480, 2);
	return EXIT_SUCCESS;
}
