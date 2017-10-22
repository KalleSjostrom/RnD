#ifdef OS_WINDOWS
	static char _msg_string[512];
	#define LOG_INFO(system, format, ...) do { \
		sprintf_s(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
		OutputDebugString(_msg_string); \
	} while(0)
	#define LOG_WARNING(system, format, ...) do { \
		sprintf_s(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
		OutputDebugString(_msg_string); \
	} while(0)
	#define LOG_ERROR(system, format, ...) do { \
		sprintf_s(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
		OutputDebugString(_msg_string); \
	} while(0)
#else
	static char _msg_string[512];
	#define LOG_INFO(system, format, ...) do { \
		printf(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
	} while(0)
	#define LOG_WARNING(system, format, ...) do { \
		printf(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
	} while(0)
	#define LOG_ERROR(system, format, ...) do { \
		printf(_msg_string, sizeof(_msg_string), format, ##__VA_ARGS__); \
	} while(0)
#endif