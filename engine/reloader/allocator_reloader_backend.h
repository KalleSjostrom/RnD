#pragma once

#include "engine/reloader/type_pairs.h"

struct AllocatorBackendHeader;

AllocatorBackendHeader *init_allocator_backend(Allocator *a, Allocator *old_allocator, Allocator *new_allocator, size_t memory_size);
bool next(AllocatorBackendHeader *header, struct AddressPair *out_address);
size_t get_count(AllocatorBackendHeader *header, size_t old_size);
void set_head(AllocatorBackendHeader *header, size_t new_size, size_t count);
void init_top(AllocatorBackendHeader *header, struct AddressPair *base);
bool is_address_in_old_range(AllocatorBackendHeader *header, intptr_t address);
void reset(AllocatorBackendHeader *header);
