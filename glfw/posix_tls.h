#ifndef _glfw3_posix_tls_h_
#define _glfw3_posix_tls_h_

#include <pthread.h>

#define _GLFW_PLATFORM_LIBRARY_TLS_STATE _GLFWtlsPOSIX posix_tls


// POSIX-specific global TLS data
//
typedef struct _GLFWtlsPOSIX
{
	pthread_key_t   context;

} _GLFWtlsPOSIX;


int _glfwCreateContextTLS(void);
void _glfwDestroyContextTLS(void);
void _glfwSetContextTLS(_GLFWwindow* context);

#endif // _glfw3_posix_tls_h_
