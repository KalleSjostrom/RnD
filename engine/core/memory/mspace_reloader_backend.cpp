#include "engine/reloader/allocator_reloader_backend.h"

struct AllocatorBackendHeader {
	malloc_state *malloc_state_old;
	malloc_state *malloc_state_new;

	malloc_chunk *current_malloc_chunk_old;
	malloc_chunk *current_malloc_chunk_new;

	intptr_t old_memory;
	intptr_t new_memory;

	size_t old_memory_size;
};

AllocatorBackendHeader *init_allocator_backend(Allocator *a, Allocator *old_allocator, Allocator *new_allocator, size_t memory_size) {
	AllocatorBackendHeader *header = (AllocatorBackendHeader *)allocate(a, sizeof(AllocatorBackendHeader));

	ASSERT(old_allocator->type == AllocatorType_MSpace, "To use mspace reload backend, the 'old' allocator needs to be mspace allocators");
	ASSERT(new_allocator->type == AllocatorType_MSpace, "To use mspace reload backend, the 'new' allocator needs to be mspace allocators");

	header->malloc_state_old = (malloc_state *)old_allocator->mspace; // 0x0010
	header->malloc_state_new = (malloc_state *)new_allocator->mspace;

	header->current_malloc_chunk_old = mem2chunk(header->malloc_state_old); // 0x0000
	header->current_malloc_chunk_new = mem2chunk(header->malloc_state_new);

	// Jump straight to the new malloc chunk
	malloc_chunk *first_user_chunk_old = next_chunk(header->current_malloc_chunk_old); // 0x03b0 (sizeof(malloc_state))
	malloc_chunk *first_user_chunk_new = next_chunk(header->current_malloc_chunk_new);

	header->old_memory = (intptr_t)chunk2mem(first_user_chunk_old); // 0x03c0
	header->new_memory = (intptr_t)chunk2mem(first_user_chunk_new);

	header->old_memory_size = memory_size;
	return header;
}

size_t _estimate_unpadded_request(size_t chunksize) {
	/////// TODO ///////
	return chunksize;
}

bool next(AllocatorBackendHeader *header, AddressPair *out_address) {
	header->current_malloc_chunk_old = next_chunk(header->current_malloc_chunk_old);
	header->current_malloc_chunk_new = next_chunk(header->current_malloc_chunk_new);

	out_address->o = (intptr_t)chunk2mem(header->current_malloc_chunk_old);
	out_address->n = (intptr_t)chunk2mem(header->current_malloc_chunk_new);

	size_t size_from_chunk = _estimate_unpadded_request(chunksize(header->current_malloc_chunk_old));
	bool done = out_address->o + size_from_chunk >= header->old_memory + header->old_memory_size;
	return done;
}

size_t get_count(AllocatorBackendHeader *header, size_t old_size) {
	size_t current_chuck_size = _estimate_unpadded_request(chunksize(header->current_malloc_chunk_old));
	size_t count = old_size == 0 ? 0 : current_chuck_size / old_size;
	return count;
}

void set_head(AllocatorBackendHeader *header, size_t new_size, size_t count) {
	// Should we instead allocate the memory here? So that we will expand into system memory if size is larger than the engines default plugin memory size?
	malloc_chunk *old_chunk = header->current_malloc_chunk_old;
	malloc_chunk *new_chunk = header->current_malloc_chunk_new;
	new_chunk->head = pad_request(new_size * count) | cinuse(old_chunk) | pinuse(old_chunk);
}

bool is_address_in_old_range(AllocatorBackendHeader *header, intptr_t address) {
	return address >= header->old_memory && address < (intptr_t)(header->old_memory + header->old_memory_size);
}

void init_top(AllocatorBackendHeader *header, AddressPair *address) {
	intptr_t used_space_new = address->n - header->new_memory;
	intptr_t used_space_old = address->o - header->old_memory;
	intptr_t size_diff = used_space_new - used_space_old;
	// If we have used more space in the new state, we have less size left to init the top with
	header->current_malloc_chunk_new->head = header->current_malloc_chunk_old->head - size_diff;
	init_top(header->malloc_state_new, header->current_malloc_chunk_new, chunksize(header->current_malloc_chunk_new));
}

// Reset state for the next iteration
void reset(AllocatorBackendHeader *header) {
	header->current_malloc_chunk_old = mem2chunk(header->malloc_state_old);
	header->current_malloc_chunk_new = mem2chunk(header->malloc_state_new);
}
