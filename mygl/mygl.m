#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#include <pthread.h>
#include <OpenGL/gl3.h>

/*static void key_callback(GLWindowHandle* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
		running = false;
	}
}*/

typedef struct MYGLmonitor
{
	CGDirectDisplayID display_id;
	CGDisplayModeRef previous_mode;
} MYGLmonitor;

typedef struct BLAH GLWindowHandle;
typedef struct MYGLWwindow
{
	MYGLmonitor *monitor;

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
} MYGLWwindow;

typedef struct MYGLlibrary
{
	MYGLWwindow *window;

	int	monitor_count;
	MYGLmonitor **monitors;

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
	MYGLWwindow* window = mygl_lib.window;
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

void mygl_init(void) {
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

		{ // MYGLmonitor** _glfwPlatformGetMonitors(int* count)
			uint32_t found = 0;
			uint32_t displayCount;
			MYGLmonitor** monitors;
			CGDirectDisplayID* displays;

			CGGetOnlineDisplayList(0, NULL, &displayCount);
			displays = calloc(displayCount, sizeof(CGDirectDisplayID));
			monitors = calloc(displayCount, sizeof(MYGLmonitor*));

			CGGetOnlineDisplayList(displayCount, displays, &displayCount);
			NSArray* screens = [NSScreen screens];

			for (uint32_t i = 0; i < displayCount; i++) {
				MYGLmonitor* monitor;

				CGDirectDisplayID screenDisplayID = CGDisplayMirrorsDisplay(displays[i]);
				if (screenDisplayID == kCGNullDirectDisplay)
					screenDisplayID = displays[i];

				NSUInteger j;
				for (j = 0; j < [screens count]; j++) {
					NSScreen* screen = [screens objectAtIndex:j];
					NSDictionary* dictionary = [screen deviceDescription];
					NSNumber* number = [dictionary objectForKey:@"NSScreenNumber"];

					if ([number unsignedIntegerValue] == screenDisplayID)
						break;
				}

				// Skip displays that has no screen
				if (j == [screens count])
					continue;

				const CGSize size = CGDisplayScreenSize(displays[i]);
				{ // _glfwAllocMonitor
					monitor = calloc(1, sizeof(MYGLmonitor));
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

GLWindowHandle* mygl_create_window(int width, int height, const char* title) {
	MYGLWwindow *window = calloc(1, sizeof(MYGLWwindow));
	mygl_lib.window = window;

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
			/*window->delegate = [[GLFWWindowDelegate alloc] initWithGlfwWindow:window];
			ASSERT(window->delegate); // "Cocoa: Failed to create window delegate"
*/
			unsigned int styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
			// styleMask = NSBorderlessWindowMask;

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

			window->view = [[NSView alloc] init];

			[window->view setWantsBestResolutionOpenGLSurface:YES];

			[window->object setTitle:[NSString stringWithUTF8String:title]];
			// [window->object setDelegate:window->delegate];
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

#if FBCONFIG_STEREO
				attributes[attribute_count++] = NSOpenGLPFAStereo;
#endif

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

			window->context =
				[[NSOpenGLContext alloc] initWithFormat:window->pixelFormat
										   shareContext:NULL];
			ASSERT(window->context);
		}

		[window->context setView:window->view];
	}

	{ // make context current
		[window->context makeCurrentContext]; // if window null: [NSOpenGLContext clearCurrentContext];
		pthread_setspecific(mygl_lib.context, window);
	}

	{ // void _glfwPlatformSwapInterval(int interval)
		int interval = 0;
		[window->context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
	}

	{ // void _glfwPlatformShowWindow(MYGLWwindow* window)
		[NSApp activateIgnoringOtherApps:YES];
		[window->object makeKeyAndOrderFront:nil];
	}

	return (GLWindowHandle*) window;
}

void mygl_set_window_title(GLWindowHandle *handle, const char *title) {
	MYGLWwindow *window = (MYGLWwindow *)handle;
	[window->object setTitle:[NSString stringWithUTF8String:title]];
}
void mygl_swap_buffers(GLWindowHandle* handle) {
	MYGLWwindow *window = (MYGLWwindow *)handle;
	[window->context flushBuffer];
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
	MYGLWwindow *window = (MYGLWwindow *)handle;

	ASSERT(window != pthread_getspecific(mygl_lib.context));

	{ // void _glfwPlatformDestroyWindow(MYGLWwindow* window)
		[window->object orderOut:nil];

		if (window->monitor) { // leaveFullscreenMode(window);
			MYGLmonitor *monitor = window->monitor;
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

		{ // void _glfwDestroyContext(MYGLWwindow* window)
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

	{ // void _glfwFreeMonitors(MYGLmonitor** monitors, int count)
		for (int i = 0;  i < mygl_lib.monitor_count;  i++)
		{ // _glfwFreeMonitor(mygl_lib.monitors[i]);
			MYGLmonitor* monitor = mygl_lib.monitors[i];
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
