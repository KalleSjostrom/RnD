//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize OpenGL support
//
int _glfwInitContextAPI(void)
{
	if (!_glfwCreateContextTLS())
        return GL_FALSE;

	_glfw.nsgl.framework =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
	if (_glfw.nsgl.framework == NULL)
	{
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "NSGL: Failed to locate OpenGL framework");
        return GL_FALSE;
	}

	return GL_TRUE;
}

// Terminate OpenGL support
//
void _glfwTerminateContextAPI(void)
{
	_glfwDestroyContextTLS();
}

// Create the OpenGL context
//
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWctxconfig* ctxconfig,
                       const _GLFWfbconfig* fbconfig)
{
	unsigned int attributeCount = 0;

	// Context robustness modes (GL_KHR_robustness) are not yet supported on
	// OS X but are not a hard constraint, so ignore and continue

	// Context release behaviors (GL_KHR_context_flush_control) are not yet
	// supported on OS X but are not a hard constraint, so ignore and continue

#define ADD_ATTR(x) { attributes[attributeCount++] = x; }
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }

	// Arbitrary array size here
	NSOpenGLPixelFormatAttribute attributes[40];

	ADD_ATTR(NSOpenGLPFAAccelerated);
	ADD_ATTR(NSOpenGLPFAClosestPolicy);

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101000
	if (ctxconfig->major >= 4)
	{
        ADD_ATTR2(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core);
	}
	else
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
	if (ctxconfig->major >= 3)
	{
        ADD_ATTR2(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core);
	}
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

	if (fbconfig->redBits != GLFW_DONT_CARE &&
        fbconfig->greenBits != GLFW_DONT_CARE &&
        fbconfig->blueBits != GLFW_DONT_CARE)
	{
        int colorBits = fbconfig->redBits +
                        fbconfig->greenBits +
                        fbconfig->blueBits;

        // OS X needs non-zero color size, so set reasonable values
        if (colorBits == 0)
            colorBits = 24;
        else if (colorBits < 15)
            colorBits = 15;

        ADD_ATTR2(NSOpenGLPFAColorSize, colorBits);
	}

	if (fbconfig->alphaBits != GLFW_DONT_CARE)
        ADD_ATTR2(NSOpenGLPFAAlphaSize, fbconfig->alphaBits);

	if (fbconfig->depthBits != GLFW_DONT_CARE)
        ADD_ATTR2(NSOpenGLPFADepthSize, fbconfig->depthBits);

	if (fbconfig->stencilBits != GLFW_DONT_CARE)
        ADD_ATTR2(NSOpenGLPFAStencilSize, fbconfig->stencilBits);

	if (fbconfig->stereo)
        ADD_ATTR(NSOpenGLPFAStereo);

	if (fbconfig->doublebuffer)
        ADD_ATTR(NSOpenGLPFADoubleBuffer);

	if (fbconfig->samples != GLFW_DONT_CARE)
	{
        if (fbconfig->samples == 0)
        {
            ADD_ATTR2(NSOpenGLPFASampleBuffers, 0);
        }
        else
        {
            ADD_ATTR2(NSOpenGLPFASampleBuffers, 1);
            ADD_ATTR2(NSOpenGLPFASamples, fbconfig->samples);
        }
	}

	// NOTE: All NSOpenGLPixelFormats on the relevant cards support sRGB
	//       framebuffer, so there's no need (and no way) to request it

	ADD_ATTR(0);

#undef ADD_ATTR
#undef ADD_ATTR2

	window->nsgl.pixelFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	if (window->nsgl.pixelFormat == nil)
	{
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "NSGL: Failed to find a suitable pixel format");
        return GL_FALSE;
	}

	NSOpenGLContext* share = NULL;

	if (ctxconfig->share)
        share = ctxconfig->share->nsgl.context;

	window->nsgl.context =
        [[NSOpenGLContext alloc] initWithFormat:window->nsgl.pixelFormat
                                   shareContext:share];
	if (window->nsgl.context == nil)
	{
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "NSGL: Failed to create OpenGL context");
        return GL_FALSE;
	}

	return GL_TRUE;
}

// Destroy the OpenGL context
//
void _glfwDestroyContext(_GLFWwindow* window)
{
	[window->nsgl.pixelFormat release];
	window->nsgl.pixelFormat = nil;

	[window->nsgl.context release];
	window->nsgl.context = nil;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
	if (window)
        [window->nsgl.context makeCurrentContext];
	else
        [NSOpenGLContext clearCurrentContext];

	_glfwSetContextTLS(window);
}

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
	// ARP appears to be unnecessary, but this is future-proof
	[window->nsgl.context flushBuffer];
}

void _glfwPlatformSwapInterval(int interval)
{
	_GLFWwindow* window = _glfwPlatformGetCurrentContext();

	GLint sync = interval;
	[window->nsgl.context setValues:&sync forParameter:NSOpenGLCPSwapInterval];
}

int _glfwPlatformExtensionSupported(const char* extension)
{
	// There are no NSGL extensions
	return GL_FALSE;
}

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
	CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                       procname,
                                                       kCFStringEncodingASCII);

	GLFWglproc symbol = CFBundleGetFunctionPointerForName(_glfw.nsgl.framework,
                                                          symbolName);

	CFRelease(symbolName);

	return symbol;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI id glfwGetNSGLContext(GLFWwindow* handle)
{
	_GLFWwindow* window = (_GLFWwindow*) handle;
	_GLFW_REQUIRE_INIT_OR_RETURN(nil);
	return window->nsgl.context;
}

