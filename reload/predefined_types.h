#pragma once

#define CUSTOM_TYPE_BASE 1000

struct PredefinedType {
	const char *name;
	int size;
};

#define MAKE_PREDEFINED_TYPE(type) { #type, sizeof(type) }

static int predefined_type_count = 7;
static PredefinedType predefined_types[] = {
	MAKE_PREDEFINED_TYPE(int),
	MAKE_PREDEFINED_TYPE(bool),
	MAKE_PREDEFINED_TYPE(unsigned),
	MAKE_PREDEFINED_TYPE(char),
	MAKE_PREDEFINED_TYPE(float),
	MAKE_PREDEFINED_TYPE(double),
	MAKE_PREDEFINED_TYPE(long),
};
/*
	predefined_types[index].name = ##type; \
	predefined_types[index].size = sizeof(type); \

*/
