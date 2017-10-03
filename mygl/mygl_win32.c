#include <gl/gl.h>

typedef struct MYGLMonitor
{
	CGDirectDisplayID display_id;
	CGDisplayModeRef previous_mode;
} MYGLMonitor;

struct InputApi {
	void (*key_down)(int key, int modifier_flags);
	void (*key_up)(int key, int modifier_flags);
};

@interface MYGLView : NSView {
	struct InputApi input_api;
}
@end

@implementation MYGLView

// - (BOOL)isOpaque { return YES; }
// - (BOOL)canBecomeKeyView { return YES; }
- (BOOL)acceptsFirstResponder { return YES; }

- (void)setInputApi:(struct InputApi*) api
{
	input_api = *api;
}

- (void)keyDown:(NSEvent *)event
{
	if ([event isARepeat])
		return;

	const int key = [event keyCode];
	const int mods = [event modifierFlags];
	NSString* characters = [event characters];

	input_api.key_down(key, mods);
}

- (void)keyUp:(NSEvent *)event
{
	const int key = [event keyCode];
	const int mods = [event modifierFlags];

	input_api.key_up(key, mods);
}

@end

typedef struct BLAH GLWindowHandle;
typedef struct MYGLWindow
{
	MYGLMonitor *monitor;

	// {_GLFWwindowNS ns;
		id object;
		id delegate;
		id view;
		unsigned int modifierFlags;
	// }
	// { _GLFWcontextNSGL nsgl;
		id pixelFormat;
		id context;
	// };
} MYGLWindow;

typedef struct MYGLlibrary
{
	MYGLWindow *window;

	int	monitor_count;
	MYGLMonitor **monitors;

	// { _GLFWlibraryNS   ns;
		CGEventSourceRef event_source;
		id autorelease_pool;
	// }
	// { _GLFWlibraryNSGL nsgl;
		void* framework;
	// }
	// { _GLFWtlsPOSIX	posix_tls;
		pthread_key_t context;
	// }
} MYGLlibrary;
MYGLlibrary mygl_lib;

@interface MyApplicationDelegate : NSObject
@end

@implementation MyApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	MYGLWindow* window = mygl_lib.window;
	// _glfwInputWindowCloseRequest(window);
	return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
	// _glfwInputMonitorChange();
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	[NSApp stop:nil];

	{ // _glfwPlatformPostEmptyEvent
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
											location:NSMakePoint(0, 0)
									   modifierFlags:0
										   timestamp:0
										windowNumber:0
											 context:nil
											 subtype:0
											   data1:0
											   data2:0];
		[NSApp postEvent:event atStart:YES];
		[pool drain];
	}
}

- (void)applicationDidHide:(NSNotification *)notification
{
	/*for (int i = 0;  i < mygl_lib.monitor_count;  i++)
		_glfwRestoreVideoMode(mygl_lib.monitors[i]);*/
}

@end

#define ASSERT(arg) { if (!(arg)) { printf("###### ASSERTION FAILED: (%s) [%s:%d]\n", (#arg), __FILE__, __LINE__); *(volatile int*)0 = 5; } }

static void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

#define FBCONFIG_REDBITS       8
#define FBCONFIG_GREENBITS     8
#define FBCONFIG_BLUEBITS      8
#define FBCONFIG_COLORBITS    (FBCONFIG_REDBITS + FBCONFIG_GREENBITS + FBCONFIG_BLUEBITS)
#define FBCONFIG_ALPHABITS     8
#define FBCONFIG_DEPTHBITS    24
#define FBCONFIG_STENCILBITS   8
#define FBCONFIG_STEREO        0
#define FBCONFIG_SAMPLES       0
#define FBCONFIG_DOUBLEBUFFER GL_TRUE

#define WNDCONFIG_RESIZABLE   GL_TRUE
#define WNDCONFIG_FLOATING    GL_FALSE

GLWindowHandle* mygl_setup(int width, int height, const char* title) {
	memset(&mygl_lib, 0, sizeof(mygl_lib));

	{ // _glfwPlatformInit
		if (!_glfwInitThreadLocalStorageWin32())
			return GLFW_FALSE;

		// To make SetForegroundWindow work as we want, we need to fiddle
		// with the FOREGROUNDLOCKTIMEOUT system setting (we do this as early
		// as possible in the hope of still being the foreground process)
		SystemParametersInfoW(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &_glfw.win32.foregroundLockTimeout, 0);
		SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, UIntToPtr(0), SPIF_SENDCHANGE);

		if (!loadLibraries())
			return GLFW_FALSE;

		createKeyTables();

		if (_glfw_SetProcessDpiAwareness)
			_glfw_SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		else if (_glfw_SetProcessDPIAware)
			_glfw_SetProcessDPIAware();

		if (!_glfwRegisterWindowClassWin32())
			return GLFW_FALSE;

		_glfw.win32.helperWindowHandle = createHelperWindow();
		if (!_glfw.win32.helperWindowHandle)
			return GLFW_FALSE;

		_glfwPlatformPollEvents();

		_glfwInitTimerWin32();
		_glfwInitJoysticksWin32();

		return GLFW_TRUE;
	}

	MYGLWindow *window = calloc(1, sizeof(MYGLWindow));
	mygl_lib.window = window;

	window->monitor = mygl_lib.monitors[0];

	// Open the actual window and create its context
	{ // _glfwPlatformCreateWindow
		if (!createNativeWindow(window, wndconfig))
			return GLFW_FALSE;

		if (ctxconfig->client != GLFW_NO_API)
		{
			if (ctxconfig->source == GLFW_NATIVE_CONTEXT_API)
			{
				if (!_glfwInitWGL())
					return GLFW_FALSE;
				if (!_glfwCreateContextWGL(window, ctxconfig, fbconfig))
					return GLFW_FALSE;
			}
			else
			{
				if (!_glfwInitEGL())
					return GLFW_FALSE;
				if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
					return GLFW_FALSE;
			}
		}

		if (window->monitor)
		{
			_glfwPlatformShowWindow(window);
			_glfwPlatformFocusWindow(window);
			if (!acquireMonitor(window))
				return GLFW_FALSE;

			centerCursor(window);
		}

		return GLFW_TRUE;
	}

	{ // make context current
		[window->context makeCurrentContext]; // if window null: [NSOpenGLContext clearCurrentContext];
		pthread_setspecific(mygl_lib.context, window);
	}

	{ // void _glfwPlatformSwapInterval(int interval)
		int interval = 2;
		[window->context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
	}

	{ // void _glfwPlatformShowWindow(MYGLWindow* window)
		[NSApp activateIgnoringOtherApps:YES];
		[window->object makeKeyAndOrderFront:nil];
	}

	return (GLWindowHandle*) window;
}

void mygl_set_input_api(GLWindowHandle *handle, struct InputApi *input_api) {
	MYGLWindow *window = (MYGLWindow *)handle;
	[window->view setInputApi:input_api];
}

void mygl_get_framebuffer_size(GLWindowHandle *handle, int* width, int* height) {
	MYGLWindow *window = (MYGLWindow *)handle;
	NSRect contentRect = [window->view frame];
	NSRect framebufferRect = [window->view convertRectToBacking:contentRect];

	if (width)
		*width = (int) framebufferRect.size.width;
	if (height)
		*height = (int) framebufferRect.size.height;
}

void mygl_enter_fullscreen_mode(GLWindowHandle *handle) {
	MYGLWindow *window = (MYGLWindow *)handle;
	CGDirectDisplayID display_id = window->monitor->display_id;
}

void mygl_set_window_title(GLWindowHandle *handle, const char *title) {
	MYGLWindow *window = (MYGLWindow *)handle;
	[window->object setTitle:[NSString stringWithUTF8String:title]];
}
void mygl_swap_buffers(GLWindowHandle* handle) {
	MYGLWindow *window = (MYGLWindow *)handle;
	[window->context flushBuffer];
}

void mygl_set_swap_interval(GLWindowHandle* handle, int interval) {
	MYGLWindow *window = (MYGLWindow *)handle;
	[window->context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
}

void mygl_poll_events(void) {
	for (;;) {
		NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
											untilDate:[NSDate distantPast]
											   inMode:NSDefaultRunLoopMode
											  dequeue:YES];
		if (event == nil)
			break;

		[NSApp sendEvent:event];
	}

	[mygl_lib.autorelease_pool drain];
	mygl_lib.autorelease_pool = [[NSAutoreleasePool alloc] init];
}

void mygl_destroy_window(GLWindowHandle *handle) {
	MYGLWindow *window = (MYGLWindow *)handle;

	ASSERT(window != pthread_getspecific(mygl_lib.context));

	{ // void _glfwPlatformDestroyWindow(MYGLWindow* window)
		[window->object orderOut:nil];

		if (window->monitor) { // leaveFullscreenMode(window);
			MYGLMonitor *monitor = window->monitor;
			if (monitor->previous_mode) {
				CGDisplayFadeReservationToken token = kCGDisplayFadeReservationInvalidToken;
				{ // beginFadeReservation
					if (CGAcquireDisplayFadeReservation(5, &token) == kCGErrorSuccess)
						CGDisplayFade(token, 0.3, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, TRUE);
				}

				CGDisplaySetDisplayMode(monitor->display_id, monitor->previous_mode, NULL);

				{ // endFadeReservation
					if (token != kCGDisplayFadeReservationInvalidToken) {
						CGDisplayFade(token, 0.5, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
						CGReleaseDisplayFadeReservation(token);
					}
				}

				CGDisplayModeRelease(monitor->previous_mode);
				monitor->previous_mode = NULL;
			}
		}

		{ // void _glfwDestroyContext(MYGLWindow* window)
			[window->pixelFormat release];
			window->pixelFormat = nil;

			[window->context release];
			window->context = nil;
		}

		[window->object setDelegate:nil];
		[window->delegate release];
		window->delegate = nil;

		[window->view release];
		window->view = nil;

		[window->object close];
		window->object = nil;
	}

	free(window);
}

void mygl_terminate() {
	if (mygl_lib.window)
		mygl_destroy_window((GLWindowHandle *) mygl_lib.window);

	{ // void _glfwFreeMonitors(MYGLMonitor** monitors, int count)
		for (int i = 0;  i < mygl_lib.monitor_count;  i++)
		{ // _glfwFreeMonitor(mygl_lib.monitors[i]);
			MYGLMonitor* monitor = mygl_lib.monitors[i];
			free(monitor);
		}

		free(mygl_lib.monitors);
	}

	{ // _glfwPlatformTerminate
		if (mygl_lib.event_source) {
			CFRelease(mygl_lib.event_source);
		}

		[mygl_lib.autorelease_pool release];

		{ // _glfwTerminateContextAPI();
			pthread_key_delete(mygl_lib.context);
		}
	}

	memset(&mygl_lib, 0, sizeof(mygl_lib));
}
