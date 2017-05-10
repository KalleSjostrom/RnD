#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#include <pthread.h>
#include <OpenGL/gl3.h>

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
		mygl_lib.autorelease_pool = [[NSAutoreleasePool alloc] init];

		mygl_lib.event_source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
		ASSERT(mygl_lib.event_source);

		CGEventSourceSetLocalEventsSuppressionInterval(mygl_lib.event_source, 0.0);

		{ // int _glfwInitContextAPI(void)
			{ // int _glfwCreateContextTLS(void)
				ASSERT(pthread_key_create(&mygl_lib.context, NULL) == 0); // "POSIX: Failed to create context TLS"
			}

			mygl_lib.framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
			ASSERT(mygl_lib.framework); // "NSGL: Failed to locate OpenGL framework"
		}

		{ // MYGLMonitor** _glfwPlatformGetMonitors(int* count)
			uint32_t found = 0;
			uint32_t display_count;
			MYGLMonitor** monitors;
			CGDirectDisplayID* displays;

			CGGetOnlineDisplayList(0, NULL, &display_count);
			displays = calloc(display_count, sizeof(CGDirectDisplayID));
			monitors = calloc(display_count, sizeof(MYGLMonitor*));

			CGGetOnlineDisplayList(display_count, displays, &display_count);
			NSArray* screens = [NSScreen screens];

			for (uint32_t i = 0; i < display_count; i++) {
				MYGLMonitor* monitor;

				CGDirectDisplayID screen_display_id = CGDisplayMirrorsDisplay(displays[i]);
				if (screen_display_id == kCGNullDirectDisplay)
					screen_display_id = displays[i];

				NSUInteger j;
				for (j = 0; j < [screens count]; j++) {
					NSScreen* screen = [screens objectAtIndex:j];
					NSDictionary* dictionary = [screen deviceDescription];
					NSNumber* number = [dictionary objectForKey:@"NSScreenNumber"];

					if ([number unsignedIntegerValue] == screen_display_id)
						break;
				}

				// Skip displays that has no screen
				if (j == [screens count])
					continue;

				const CGSize size = CGDisplayScreenSize(displays[i]);
				{ // _glfwAllocMonitor
					monitor = calloc(1, sizeof(MYGLMonitor));
					/*monitor->name = "no_name (dep on IOKit)";
					monitor->widthMM = size.width;
					monitor->heightMM = size.height;*/
				}
				monitor->display_id = displays[i];

				found++;
				monitors[found - 1] = monitor;
			}

			free(displays);

			mygl_lib.monitor_count = found;
			mygl_lib.monitors = monitors;
		}
	}

	MYGLWindow *window = calloc(1, sizeof(MYGLWindow));
	mygl_lib.window = window;

	window->monitor = mygl_lib.monitors[0];

	// Open the actual window and create its context
	{ // _glfwPlatformCreateWindow
		{ // initializeAppKit
			ASSERT(!NSApp);

			// Implicitly create shared NSApplication instance
			[NSApplication sharedApplication];

			// In case we are unbundled, make us a proper UI application
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

			// There can only be one application delegate, but we allocate it the
			// first time a window is created to keep all window code in this file
			id delegate = [[MyApplicationDelegate alloc] init];
			ASSERT(delegate) // "Cocoa: Failed to create application delegate"

			[NSApp setDelegate:delegate];
			[NSApp run];
		}

		{ // createWindow(_GLFWwindow* window, const _GLFWwndconfig* wndconfig)
			unsigned int styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

			if (WNDCONFIG_RESIZABLE)
				styleMask |= NSWindowStyleMaskResizable;

			NSRect contentRect = NSMakeRect(0, 0, width, height);

			window->object = [[NSWindow alloc]
				initWithContentRect:contentRect
						  styleMask:styleMask
							backing:NSBackingStoreBuffered
							  defer:NO];

			ASSERT(window->object) // "Cocoa: Failed to create window"

			if (WNDCONFIG_RESIZABLE)
				[window->object setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

			[window->object center];
			if (WNDCONFIG_FLOATING)
				[window->object setLevel:NSFloatingWindowLevel];

			window->view = [[MYGLView alloc] init];

			[window->view setWantsBestResolutionOpenGLSurface:YES];

			[window->object setTitle:[NSString stringWithUTF8String:title]];
			[window->object setAcceptsMouseMovedEvents:YES];
			[window->object setContentView:window->view];

			[window->object setRestorable:NO];
		}

		{ // _glfwCreateContext
			unsigned int attribute_count = 0;
			NSOpenGLPixelFormatAttribute attributes[40];

			attributes[attribute_count++] = NSOpenGLPFAAccelerated;
			attributes[attribute_count++] = NSOpenGLPFAClosestPolicy;

			attributes[attribute_count++] = NSOpenGLPFAOpenGLProfile; // MAJOR >= 4
			attributes[attribute_count++] = NSOpenGLProfileVersion4_1Core; // MAJOR >= 4

			attributes[attribute_count++] = NSOpenGLPFAColorSize;
			attributes[attribute_count++] = FBCONFIG_COLORBITS;

			attributes[attribute_count++] = NSOpenGLPFAAlphaSize;
			attributes[attribute_count++] = FBCONFIG_ALPHABITS;

			attributes[attribute_count++] = NSOpenGLPFADepthSize;
			attributes[attribute_count++] = FBCONFIG_DEPTHBITS;

			attributes[attribute_count++] = NSOpenGLPFAStencilSize;
			attributes[attribute_count++] = FBCONFIG_STENCILBITS;

			if (FBCONFIG_STEREO) {
				// "NSOpenGLPFAStereo" is deprecated in the 10.12 SDK, suppress warning about its use.
				// No explanation is given for the deprecation, and no alternative is suggested.
				#pragma clang diagnostic push
				#pragma clang diagnostic ignored "-Wdeprecated-declarations"
				attributes[attribute_count++] = NSOpenGLPFAStereo;
				#pragma clang diagnostic pop
			}

			if (FBCONFIG_DOUBLEBUFFER)
				attributes[attribute_count++] = NSOpenGLPFADoubleBuffer;

			if (FBCONFIG_SAMPLES != -1) {
				if (FBCONFIG_SAMPLES == 0) {
					attributes[attribute_count++] = NSOpenGLPFASampleBuffers;
					attributes[attribute_count++] = 0;
				} else {
					attributes[attribute_count++] = NSOpenGLPFASampleBuffers;
					attributes[attribute_count++] = 1;

					attributes[attribute_count++] = NSOpenGLPFASamples;
					attributes[attribute_count++] = FBCONFIG_SAMPLES;
				}
			}

			attributes[attribute_count++] = 0;

			window->pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
			ASSERT(window->pixelFormat);

			window->context = [[NSOpenGLContext alloc] initWithFormat:window->pixelFormat shareContext:NULL];
			ASSERT(window->context);
		}

		[window->context setView:window->view];
	}

	{ // make context current
		[window->context makeCurrentContext]; // if window null: [NSOpenGLContext clearCurrentContext];
		pthread_setspecific(mygl_lib.context, window);
	}

	{ // void _glfwPlatformSwapInterval(int interval)
		int interval = 1;
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

// Enter full screen mode
//
/*
GLboolean _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
	CFArrayRef modes;
	CFIndex count, i;
	CVDisplayLinkRef link;
	CGDisplayModeRef native = NULL;
	GLFWvidmode current;
	const GLFWvidmode* best;

	best = _glfwChooseVideoMode(monitor, desired);
	_glfwPlatformGetVideoMode(monitor, &current);
	if (_glfwCompareVideoModes(&current, best) == 0)
		return GL_TRUE;

	CVDisplayLinkCreateWithCGDisplay(monitor->ns.display_id, &link);

	modes = CGDisplayCopyAllDisplayModes(monitor->ns.display_id, NULL);
	count = CFArrayGetCount(modes);

	for (i = 0;  i < count;  i++)
	{
		CGDisplayModeRef dm = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
		if (!modeIsGood(dm))
			continue;

		const GLFWvidmode mode = vidmodeFromCGDisplayMode(dm, link);
		if (_glfwCompareVideoModes(best, &mode) == 0)
		{
			native = dm;
			break;
		}
	}

	if (native)
	{
		if (monitor->ns.previousMode == NULL)
			monitor->ns.previousMode = CGDisplayCopyDisplayMode(monitor->ns.display_id);

		CGDisplayFadeReservationToken token = beginFadeReservation();
		CGDisplaySetDisplayMode(monitor->ns.display_id, native, NULL);
		endFadeReservation(token);
	}

	CFRelease(modes);
	CVDisplayLinkRelease(link);

	if (!native)
	{
		_glfwInputError(GLFW_PLATFORM_ERROR,
						"Cocoa: Monitor mode list changed");
		return GL_FALSE;
	}

	return GL_TRUE;
}
void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode *mode)
{
	CGDisplayModeRef displayMode;
	CVDisplayLinkRef link;
	CVDisplayLinkCreateWithCGDisplay(monitor->ns.display_id, &link);
	displayMode = CGDisplayCopyDisplayMode(monitor->ns.display_id);
	*mode = vidmodeFromCGDisplayMode(displayMode, link);
	CGDisplayModeRelease(displayMode);
	CVDisplayLinkRelease(link);
}
void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
	const CGRect bounds = CGDisplayBounds(monitor->ns.display_id);

	if (xpos)
		*xpos = (int) bounds.origin.x;
	if (ypos)
		*ypos = (int) bounds.origin.y;
}
static GLboolean enterFullscreenMode(_GLFWwindow* window) {
	GLFWvidmode mode;
	GLboolean status;
	int xpos, ypos;

	status = _glfwSetVideoMode(window->monitor, &window->videoMode);

	_glfwPlatformGetVideoMode(window->monitor, &mode);
	_glfwPlatformGetMonitorPos(window->monitor, &xpos, &ypos);

	[window->ns.object setFrame:NSMakeRect(xpos, ypos, mode.width, mode.height)
						display:YES];

	return status;
}

void _glfwRestoreVideoMode(_GLFWmonitor* monitor)
{
	if (monitor->ns.previousMode)
	{
		CGDisplayFadeReservationToken token = beginFadeReservation();
		CGDisplaySetDisplayMode(monitor->ns.display_id,
								monitor->ns.previousMode, NULL);
		endFadeReservation(token);

		CGDisplayModeRelease(monitor->ns.previousMode);
		monitor->ns.previousMode = NULL;
	}
}

static void leaveFullscreenMode(GLWindowHandle *handle) {
	MYGLWindow *window = (MYGLWindow *)handle;
	_glfwRestoreVideoMode(window->monitor);
}
// Check whether the display mode should be included in enumeration
static GLboolean modeIsGood(CGDisplayModeRef mode)
{
	uint32_t flags = CGDisplayModeGetIOFlags(mode);
	if (!(flags & kDisplayModeValidFlag) || !(flags & kDisplayModeSafeFlag))
	    return GL_FALSE;

	if (flags & kDisplayModeInterlacedFlag)
	    return GL_FALSE;

	if (flags & kDisplayModeStretchedFlag)
	    return GL_FALSE;

	return GL_TRUE;
}
*/

void mygl_enter_fullscreen_mode(GLWindowHandle *handle) {
	MYGLWindow *window = (MYGLWindow *)handle;
	CGDirectDisplayID display_id = window->monitor->display_id;

	// CGDisplayCapture(display_id);

	/*
	CFIndex count, i;
	GLFWvidmode current;
	const GLFWvidmode* best;*/

	// CVDisplayLinkRef link;
	// CVDisplayLinkCreateWithCGDisplay(display_id, &link);

	// CGDisplayModeRef native = NULL;

	// CFArrayRef modes = CGDisplayCopyAllDisplayModes(display_id, NULL);
	// CFIndex count = CFArrayGetCount(modes);

	// for (CFIndex i = 0;  i < count;  i++) {
	// 	CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);

	// 	int width = (int) CGDisplayModeGetWidth(mode);
	// 	int height = (int) CGDisplayModeGetHeight(mode);
	// 	int refresh_rate = (int) CGDisplayModeGetRefreshRate(mode);

	// 	// if (refresh_rate == 0) {
	// 	//     const CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(link);
	// 	//     if (!(time.flags & kCVTimeIsIndefinite))
	// 	//         refresh_rate = (int) (time.timeScale / (double) time.timeValue);
	// 	// }

	// 	int a = 6;
	// 	printf("%d %d %d\n", width, height, refresh_rate);

	// 	// if (!modeIsGood(dm))
	// 	//     continue;

	// 	// const GLFWvidmode mode = vidmodeFromCGDisplayMode(dm, link);
	// 	// if (_glfwCompareVideoModes(best, &mode) == 0)
	// 	// {
	// 	//     native = dm;
	// 	//     break;
	// 	// }
	// }

	// if (native) {
	// 	window->monitor->previous_mode = CGDisplayCopyDisplayMode(display_id);

	// 	// CGDisplayFadeReservationToken token = beginFadeReservation();
	// 	CGDisplaySetDisplayMode(display_id, native, NULL);
	// 	// endFadeReservation(token);
	// }

	// CFRelease(modes);
	// CVDisplayLinkRelease(link);
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

void mygl_poll_events(void)
{
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
