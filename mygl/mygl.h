#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <OpenGL/gl3.h>

typedef struct BLAH GLWindowHandle;

void mygl_init();
void mygl_terminate();

GLWindowHandle* mygl_create_window(int width, int height, const char* title);
void mygl_destroy_window(GLWindowHandle* handle);

void mygl_set_window_title(GLWindowHandle *handle, const char *title);

void mygl_swap_buffers(GLWindowHandle* handle);
void mygl_poll_events(void);

#ifdef __cplusplus
}
#endif