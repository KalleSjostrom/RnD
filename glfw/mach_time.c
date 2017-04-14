#include <mach/mach_time.h>


// Return raw time
//
static uint64_t getRawTime(void)
{
	return mach_absolute_time();
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialise timer
//
void _glfwInitTimer(void)
{
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);

	_glfw.ns_time.resolution = (double) info.numer / (info.denom * 1.0e9);
	_glfw.ns_time.base = getRawTime();
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

double _glfwPlatformGetTime(void)
{
	return (double) (getRawTime() - _glfw.ns_time.base) *
	    _glfw.ns_time.resolution;
}

void _glfwPlatformSetTime(double time)
{
	_glfw.ns_time.base = getRawTime() -
	    (uint64_t) (time / _glfw.ns_time.resolution);
}

