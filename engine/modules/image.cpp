#include "common.h"

#include "include/SDL_Image.h"

#include "engine/modules/logging.h"

#include "image_api.h"
#include "image.h"

void image_init() {
	if (!IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG)) {
		// ASSERT(0, "IMG_Init failed: %s", SDL_GetError());
	}
}

void image_deinit() {
	IMG_Quit();
}

bool image_load(const char *filepath, ImageData &data) {
	SDL_Surface *surface = IMG_Load(filepath);
	if (!surface) {
		log_info("Engine", "Image load failed: %s\n", IMG_GetError());
		return false;
	}

	log_info("Engine", "Loading image '%s'. Pixel format: %s\n", filepath, SDL_GetPixelFormatName(surface->format->format));

	data.pixels = surface->pixels;
	data.bytes_per_pixel = surface->format->BytesPerPixel;
	data.width = surface->w;
	data.height = surface->h;

	switch (surface->format->format) {
		case SDL_PIXELFORMAT_RGBA32: { data.format = PixelFormat_RGBA; } break;
		case SDL_PIXELFORMAT_BGRA32: { data.format = PixelFormat_ARGB; } break;
		case SDL_PIXELFORMAT_RGB24: { data.format = PixelFormat_RGB; } break;
		case SDL_PIXELFORMAT_RGB888: { data.format = PixelFormat_RGB; } break;
		case SDL_PIXELFORMAT_BGR24: { data.format = PixelFormat_BGR; } break;
		case SDL_PIXELFORMAT_UNKNOWN: { log_info("Engine", "SDL_PIXELFORMAT_UNKNOWN\n"); } break;
		case SDL_PIXELFORMAT_INDEX1LSB: { log_info("Engine", "SDL_PIXELFORMAT_INDEX1LSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX1MSB: { log_info("Engine", "SDL_PIXELFORMAT_INDEX1MSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX4LSB: { log_info("Engine", "SDL_PIXELFORMAT_INDEX4LSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX4MSB: { log_info("Engine", "SDL_PIXELFORMAT_INDEX4MSB\n"); } break;
		case SDL_PIXELFORMAT_INDEX8: { log_info("Engine", "SDL_PIXELFORMAT_INDEX8\n"); } break;
		case SDL_PIXELFORMAT_RGB332: { log_info("Engine", "SDL_PIXELFORMAT_RGB332\n"); } break;
		case SDL_PIXELFORMAT_RGB444: { log_info("Engine", "SDL_PIXELFORMAT_RGB444\n"); } break;
		case SDL_PIXELFORMAT_RGB555: { log_info("Engine", "SDL_PIXELFORMAT_RGB555\n"); } break;
		case SDL_PIXELFORMAT_BGR555: { log_info("Engine", "SDL_PIXELFORMAT_BGR555\n"); } break;
		case SDL_PIXELFORMAT_ARGB4444: { log_info("Engine", "SDL_PIXELFORMAT_ARGB4444\n"); } break;
		case SDL_PIXELFORMAT_RGBA4444: { log_info("Engine", "SDL_PIXELFORMAT_RGBA4444\n"); } break;
		case SDL_PIXELFORMAT_ABGR4444: { log_info("Engine", "SDL_PIXELFORMAT_ABGR4444\n"); } break;
		case SDL_PIXELFORMAT_BGRA4444: { log_info("Engine", "SDL_PIXELFORMAT_BGRA4444\n"); } break;
		case SDL_PIXELFORMAT_ARGB1555: { log_info("Engine", "SDL_PIXELFORMAT_ARGB1555\n"); } break;
		case SDL_PIXELFORMAT_RGBA5551: { log_info("Engine", "SDL_PIXELFORMAT_RGBA5551\n"); } break;
		case SDL_PIXELFORMAT_ABGR1555: { log_info("Engine", "SDL_PIXELFORMAT_ABGR1555\n"); } break;
		case SDL_PIXELFORMAT_BGRA5551: { log_info("Engine", "SDL_PIXELFORMAT_BGRA5551\n"); } break;
		case SDL_PIXELFORMAT_RGB565: { log_info("Engine", "SDL_PIXELFORMAT_RGB565\n"); } break;
		case SDL_PIXELFORMAT_BGR565: { log_info("Engine", "SDL_PIXELFORMAT_BGR565\n"); } break;
		case SDL_PIXELFORMAT_RGBX8888: { log_info("Engine", "SDL_PIXELFORMAT_RGBX8888\n"); } break;
		case SDL_PIXELFORMAT_BGR888: { log_info("Engine", "SDL_PIXELFORMAT_BGR888\n"); } break;
		case SDL_PIXELFORMAT_BGRX8888: { log_info("Engine", "SDL_PIXELFORMAT_BGRX8888\n"); } break;
		case SDL_PIXELFORMAT_ARGB2101010: { log_info("Engine", "SDL_PIXELFORMAT_ARGB2101010\n"); } break;
		case SDL_PIXELFORMAT_YV12: { log_info("Engine", "SDL_PIXELFORMAT_YV12\n"); } break;
		case SDL_PIXELFORMAT_IYUV: { log_info("Engine", "SDL_PIXELFORMAT_IYUV\n"); } break;
		case SDL_PIXELFORMAT_YUY2: { log_info("Engine", "SDL_PIXELFORMAT_YUY2\n"); } break;
		case SDL_PIXELFORMAT_UYVY: { log_info("Engine", "SDL_PIXELFORMAT_UYVY\n"); } break;
		case SDL_PIXELFORMAT_YVYU: { log_info("Engine", "SDL_PIXELFORMAT_YVYU\n"); } break;
		case SDL_PIXELFORMAT_NV12: { log_info("Engine", "SDL_PIXELFORMAT_NV12\n"); } break;
		case SDL_PIXELFORMAT_NV21		: { log_info("Engine", "SDL_PIXELFORMAT_NV21\n"); } break;
		default: {
			// ASSERT(0, "Unsuported pixel format!");
			return false;
		}
	};

	return true;
}
