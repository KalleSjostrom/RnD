#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// #include <OpenGL/gl3.h>

struct InputApi {
	void (*key_down)(int key, int modifier_flags);
	void (*key_up)(int key, int modifier_flags);
};

typedef struct BLAH GLWindowHandle;

void mygl_terminate();

GLWindowHandle* mygl_setup(int width, int height, const char* title);
void mygl_destroy_window(GLWindowHandle* handle);

void mygl_set_input_api(GLWindowHandle *handle, InputApi *input_api);

void mygl_get_framebuffer_size(GLWindowHandle *handle, int* width, int* height);
void mygl_set_window_title(GLWindowHandle *handle, const char *title);

void mygl_swap_buffers(GLWindowHandle* handle);
void mygl_set_swap_interval(GLWindowHandle* handle, int interval);
void mygl_poll_events(void);

void mygl_enter_fullscreen_mode(GLWindowHandle* handle);

#ifdef __cplusplus
}
#endif
