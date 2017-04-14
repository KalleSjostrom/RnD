#ifndef _glfw3_nsgl_context_h_
#define _glfw3_nsgl_context_h_

#define _GLFW_PLATFORM_FBCONFIG
#define _GLFW_PLATFORM_CONTEXT_STATE            _GLFWcontextNSGL nsgl
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE    _GLFWlibraryNSGL nsgl


// NSGL-specific per-context data
//
typedef struct _GLFWcontextNSGL
{
	id           pixelFormat;
	id	         context;

} _GLFWcontextNSGL;


// NSGL-specific global data
//
typedef struct _GLFWlibraryNSGL
{
	// dlopen handle for OpenGL.framework (for glfwGetProcAddress)
	void*           framework;

} _GLFWlibraryNSGL;


int _glfwInitContextAPI(void);
void _glfwTerminateContextAPI(void);
int _glfwCreateContext(_GLFWwindow* window,
	                   const _GLFWctxconfig* ctxconfig,
	                   const _GLFWfbconfig* fbconfig);
void _glfwDestroyContext(_GLFWwindow* window);

#endif // _glfw3_nsgl_context_h_
