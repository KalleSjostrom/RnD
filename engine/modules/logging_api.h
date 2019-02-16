#pragma once

struct LoggingApi {
	void (*log_info)(const char *system, const char *format, ...);
	void (*log_warning)(const char *system, const char *format, ...);
	void (*log_error)(const char *system, const char *format, ...);
};

#if defined(PLUGIN)
	static void (*log_info)(const char *system, const char *format, ...);
	static void (*log_warning)(const char *system, const char *format, ...);
	static void (*log_error)(const char *system, const char *format, ...);

	void global_set_logging(LoggingApi &error) {
		log_info    = error.log_info;
		log_warning = error.log_warning;
		log_error   = error.log_error;
	}
#endif
