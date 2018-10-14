#pragma once

enum PixelFormat {
	PixelFormat_RGBA,
	PixelFormat_ARGB,
	PixelFormat_RGB,
	PixelFormat_BGR,
};

struct ImageData {
	void *pixels;

	int bytes_per_pixel;
	int width, height;
	PixelFormat format;
};

struct ImageApi {
	bool (*load)(const char *filename, ImageData &image_data);
};
