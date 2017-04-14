// The three global variables below comprise all global data in GLFW.
// Any other global variable is a bug.

// Global state shared between compilation units of GLFW
// These are documented in internal.h
//
GLboolean _glfwInitialized = GL_FALSE;
_GLFWlibrary _glfw;

// This is outside of _glfw so it can be initialized and usable before
// glfwInit is called, which lets that function report errors
//
static GLFWerrorfun _glfwErrorCallback = NULL;


// Returns a generic string representation of the specified error
//
static const char* getErrorString(int error)
{
	switch (error)
	{
	    case GLFW_NOT_INITIALIZED:
	        return "The GLFW library is not initialized";
	    case GLFW_NO_CURRENT_CONTEXT:
	        return "There is no current context";
	    case GLFW_INVALID_ENUM:
	        return "Invalid argument for enum parameter";
	    case GLFW_INVALID_VALUE:
	        return "Invalid value for parameter";
	    case GLFW_OUT_OF_MEMORY:
	        return "Out of memory";
	    case GLFW_API_UNAVAILABLE:
	        return "The requested client API is unavailable";
	    case GLFW_VERSION_UNAVAILABLE:
	        return "The requested client API version is unavailable";
	    case GLFW_PLATFORM_ERROR:
	        return "A platform-specific error occurred";
	    case GLFW_FORMAT_UNAVAILABLE:
	        return "The requested format is unavailable";
	}

	return "ERROR: UNKNOWN ERROR TOKEN PASSED TO glfwErrorString";
}


//////////////////////////////////////////////////////////////////////////
//////                         GLFW event API                       //////
//////////////////////////////////////////////////////////////////////////

void _glfwInputError(int error, const char* format, ...)
{
	if (_glfwErrorCallback)
	{
	    char buffer[8192];
	    const char* description;

	    if (format)
	    {
	        int count;
	        va_list vl;

	        va_start(vl, format);
	        count = vsnprintf(buffer, sizeof(buffer), format, vl);
	        va_end(vl);

	        if (count < 0)
	            buffer[sizeof(buffer) - 1] = '\0';

	        description = buffer;
	    }
	    else
	        description = getErrorString(error);

	    _glfwErrorCallback(error, description);
	}
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwInit(void)
{
	if (_glfwInitialized)
	    return GL_TRUE;

	memset(&_glfw, 0, sizeof(_glfw));

	if (!_glfwPlatformInit())
	{
	    _glfwPlatformTerminate();
	    return GL_FALSE;
	}

	_glfw.monitors = _glfwPlatformGetMonitors(&_glfw.monitorCount);
	_glfwInitialized = GL_TRUE;

	// Not all window hints have zero as their default value
	glfwDefaultWindowHints();

	return GL_TRUE;
}

GLFWAPI void glfwTerminate(void)
{
	int i;

	if (!_glfwInitialized)
	    return;

	memset(&_glfw.callbacks, 0, sizeof(_glfw.callbacks));

	while (_glfw.windowListHead)
	    glfwDestroyWindow((GLFWwindow*) _glfw.windowListHead);

	while (_glfw.cursorListHead)
	    glfwDestroyCursor((GLFWcursor*) _glfw.cursorListHead);

	for (i = 0;  i < _glfw.monitorCount;  i++)
	{
	    _GLFWmonitor* monitor = _glfw.monitors[i];
	    if (monitor->originalRamp.size)
	        _glfwPlatformSetGammaRamp(monitor, &monitor->originalRamp);
	}

	_glfwFreeMonitors(_glfw.monitors, _glfw.monitorCount);
	_glfw.monitors = NULL;
	_glfw.monitorCount = 0;

	_glfwPlatformTerminate();

	memset(&_glfw, 0, sizeof(_glfw));
	_glfwInitialized = GL_FALSE;
}

GLFWAPI GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun cbfun)
{
	_GLFW_SWAP_POINTERS(_glfwErrorCallback, cbfun);
	return cbfun;
}

