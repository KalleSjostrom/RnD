#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef offsetof
#define offsetof(st, m) ((size_t)(&((st *)0)->m))
#endif

struct AddressPair {
	intptr_t addr_old;
	intptr_t addr_new;
};

struct MemorySlot {
	unsigned type;
	intptr_t addr_new;
	AddressPair *target;
	intptr_t target_addr_old;
	unsigned size;
};

struct MemoryMapper {
	AddressPair *address_lookup;
	unsigned address_lookup_size;
	MemorySlot *memory_slots;
	unsigned memory_slot_counter;
};

#define ADDRESS_LOOKUP_SIZE 1024
AddressPair *lookup(AddressPair *address_lookup, intptr_t addr_old) {
	for (size_t i = 0; i < ADDRESS_LOOKUP_SIZE; ++i) {
		size_t hash_index = ((addr_old + i) % ADDRESS_LOOKUP_SIZE);
		AddressPair *entry = address_lookup + hash_index;
		if (entry->addr_old == addr_old || entry->addr_old == 0)
			return entry;
	}

	return 0;
}
void remove(AddressPair *address_lookup, intptr_t addr_old) {
	for (size_t i = 0; i < ADDRESS_LOOKUP_SIZE; ++i) {
		size_t hash_index = ((addr_old + i) % ADDRESS_LOOKUP_SIZE);
		AddressPair *entry = address_lookup + hash_index;
		if (entry->addr_old == addr_old)
			entry->addr_old = 0;
	}
}

inline void set_bool(MemoryMapper *mapper, intptr_t addr_old, intptr_t addr_new) {
	AddressPair *entry = lookup(mapper->address_lookup, addr_old);
	if (entry->addr_old == 0) {
		entry->addr_old = addr_old;
		entry->addr_new = addr_new;
		*(bool*)(entry->addr_new) = *(bool*)(entry->addr_old);
	}
}

inline void set_int(MemoryMapper *mapper, intptr_t addr_old, intptr_t addr_new) {
	AddressPair *entry = lookup(mapper->address_lookup, addr_old);
	if (entry->addr_old == 0) {
		entry->addr_old = addr_old;
		entry->addr_new = addr_new;
		*(int*)(entry->addr_new) = *(int*)(entry->addr_old);
	}
}

inline void set_float(MemoryMapper *mapper, intptr_t addr_old, intptr_t addr_new) {
	AddressPair *entry = lookup(mapper->address_lookup, addr_old);
	if (entry->addr_old == 0) {
		entry->addr_old = addr_old;
		entry->addr_new = addr_new;
		*(float*)(entry->addr_new) = *(float*)(entry->addr_old);
	}
}

inline void set_generic(MemoryMapper *mapper, intptr_t addr_old, intptr_t addr_new, size_t size) {
	AddressPair *entry = lookup(mapper->address_lookup, addr_old);
	if (entry->addr_old == 0) {
		entry->addr_old = addr_old;
		entry->addr_new = addr_new;
		for (int i = 0; i < size; ++i) {
			*(char*)(entry->addr_new + i) = *(char*)(entry->addr_old + i);
		}
	}
}

inline void set_pointer(MemoryMapper *mapper, intptr_t addr_old, intptr_t addr_new, unsigned pointer_type, unsigned size) {
	AddressPair *entry = lookup(mapper->address_lookup, addr_old);
	if (entry->addr_old == 0) {
		entry->addr_old = addr_old;
		entry->addr_new = addr_new;

		intptr_t target_addr_old = *(intptr_t*)(addr_old); // chase the pointer to find the targt in the old space.
		if (target_addr_old != 0) {
			AddressPair *target = lookup(mapper->address_lookup, target_addr_old); // do we have a mapping for that memory?

			if (target->addr_old == 0) {
				MemorySlot memory_slot = {0};

				memory_slot.type            = pointer_type;
				memory_slot.addr_new        = entry->addr_new;
				memory_slot.target          = target;
				memory_slot.target_addr_old = target_addr_old;
				memory_slot.size            = size;

				// found no mapping between target address in old space to new space.
				mapper->memory_slots[mapper->memory_slot_counter++] = memory_slot; // need to memory_slots the pointer up for later.
			} else {
				*(intptr_t*)(entry->addr_new) = target->addr_new; // we have a mapping, and we assign the pointer to the correct place in new memory.
			}
		} else {
			*(intptr_t*)(entry->addr_new) = 0;
		}
	} else { // TODO: Default values of new pointers? Only in debug?
		*(intptr_t*)(entry->addr_new) = 0;
	}
}
