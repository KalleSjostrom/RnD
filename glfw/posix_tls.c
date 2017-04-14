//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwCreateContextTLS(void)
{
	if (pthread_key_create(&_glfw.posix_tls.context, NULL) != 0)
	{
	    _glfwInputError(GLFW_PLATFORM_ERROR,
	                    "POSIX: Failed to create context TLS");
	    return GL_FALSE;
	}

	return GL_TRUE;
}

void _glfwDestroyContextTLS(void)
{
	pthread_key_delete(_glfw.posix_tls.context);
}

void _glfwSetContextTLS(_GLFWwindow* context)
{
	pthread_setspecific(_glfw.posix_tls.context, context);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
	return pthread_getspecific(_glfw.posix_tls.context);
}

