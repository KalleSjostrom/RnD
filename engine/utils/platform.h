#pragma once

/// Check operating system
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	#define OS_WINDOWS
#elif defined(__APPLE__)
	#include "TargetConditionals.h"

	#if defined(TARGET_OS_MAC)
		#define OS_MAC
	#elif defined(TARGET_OS_IPHONE)
		#define OS_iOS
	#else
		#error Unknown Apple platform
	#endif
#elif defined(__linux__)
	#define OS_LINUX
#endif
