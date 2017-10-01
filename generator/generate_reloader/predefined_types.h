#pragma once

struct PredefinedType {
	const char *name;
	i32 size;
	i32 __padding;
};

#define MAKE_PREDEFINED_TYPE(type) { #type, sizeof(type), 0 }

static PredefinedType predefined_types[] = {
	MAKE_PREDEFINED_TYPE(int),
	MAKE_PREDEFINED_TYPE(bool),
	MAKE_PREDEFINED_TYPE(unsigned),
	MAKE_PREDEFINED_TYPE(char),
	MAKE_PREDEFINED_TYPE(float),
	MAKE_PREDEFINED_TYPE(double),
	MAKE_PREDEFINED_TYPE(long),
};
