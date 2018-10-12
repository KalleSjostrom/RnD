#include "dynamic_string.h"

DynamicString dynamic_stringf(Allocator *a, const char *format, ...) {
	va_list args;
	va_start(args, format);
	int length = vsnprintf(0, 0, format, args) + 1;
	va_end(args);

	DynamicString ds = dynamic_string(a, length);

	va_start(args, format);
	vsnprintf(ds.text, length, format, args);
	va_end(args);

	array_count(ds.text) += length - 1;
	return ds;
}

void printf(DynamicString &ds, const char *format, ...) {
	va_list args;
	va_start(args, format);
	int length = vsnprintf(0, 0, format, args) + 1;
	va_end(args);

	int current_length = string_length(ds);
	_array_ensure_space(ds.text, length, 0);

	va_start(args, format);
	vsnprintf(ds.text + current_length, length, format, args);
	va_end(args);

	array_count(ds.text) += length - 1; // we don't want the null terminator here
}
