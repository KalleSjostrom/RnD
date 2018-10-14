#pragma once

void log_init();
void log_update();

void log_info(const char *system, const char *format, ...);
void log_warning(const char *system, const char *format, ...);
void log_error(const char *system, const char *format, ...);
