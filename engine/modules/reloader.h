#pragma once

struct ReloadHeader {
	struct Allocator *old_allocator;
	struct Allocator *new_allocator;
	size_t memory_size;
};

void *reloader_setup(struct Allocator *a, const char *pdb_name, const char *entry_name);
int reloader_reload(struct Allocator *a, const char *pdb_name, ReloadHeader *reload_header, void *handle);
