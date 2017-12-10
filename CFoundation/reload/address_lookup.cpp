struct MappingRegion {
	intptr_t addr_old;
	size_t size_old;

	intptr_t addr_new;
	size_t size_new;

	intptr_t pointer_type;

#if DEBUG_RELOAD
	char *debug_tag;
#endif // DEBUG_RELOAD
};

struct AddressNode {
	MappingRegion *region;
	AddressNode *next;
};

struct RegionHashEntry {
	intptr_t key;
	MappingRegion *value;
};

struct AddressLookup {
	AddressNode *node_storage;
	unsigned node_storage_count;

	MappingRegion *region_storage;
	unsigned region_storage_count;

	AddressNode **buckets;

	RegionHashEntry region_hash[MAX_MAPPED_REGIONS];

	intptr_t memory_base;
	size_t   memory_size;
};

__forceinline unsigned get_bucket_index(AddressLookup *lookup, intptr_t address) {
	return (unsigned)((address - lookup->memory_base) / BUCKET_MEMORY_RANGE);
}

// This assumes that the address is within the reloadable memory block!
intptr_t get_address_to_block(AddressLookup *lookup, intptr_t address, intptr_t pointer_type) {
	{ // Search the hash map
		RegionHashEntry *map = lookup->region_hash;
		unsigned size = MAX_MAPPED_REGIONS;
		unsigned index = address & size;
		RegionHashEntry *entry;
		for (unsigned i = 0; i < size; ++i) {
			entry = map + index;
			if (entry->key == address) { // Look for a matching slot
				// We found a region for the given address, compare pointer_type to see if it's the desired region

				// If we are pointing to void*, we don't have any information about what we should point to so just return the first region
				if (pointer_type == __RELOAD_TYPE__any) {
					// TODO(kalle): What region to return on any? The largest?
					return entry->value->addr_new;
				} else if (pointer_type == entry->value->pointer_type) {
					return entry->value->addr_new;
				}
			} else if (entry->key == 0) { // Since we don't ever remove stuff from the hash-map, if this is empty then we can't find what we are looking for.
				break;
			}
			index++;
			if (index == size)
				index = 0;
		}
	}

	return 0;
}

// This assumes that the address is within the reloadable memory block!
intptr_t get_address_inside_block(AddressLookup *lookup, intptr_t address, intptr_t pointer_type) {
	{ // Lookup the region in the node storage
		unsigned bucket_index = get_bucket_index(lookup, address);
		AddressNode *address_node = lookup->buckets[bucket_index];
		while (address_node) {
			intptr_t diff = address - address_node->region->addr_old;
			if (diff >= 0 && diff < (intptr_t)address_node->region->size_old) {
				if (pointer_type == 0 || pointer_type == address_node->region->pointer_type)
					return address_node->region->addr_new + diff;
			}
			address_node = address_node->next;
		}
	}

	return 0;
}

// Adds a memory mapping region from [addr_old, addr_old + size_old] -> [addr_new, addr_new + size_new]
void add_node_to_lookup(AddressLookup *lookup, intptr_t addr_old, size_t size_old, intptr_t addr_new, size_t size_new, intptr_t pointer_type, char *debug_tag = 0) {
	ASSERT(lookup->region_storage_count < MAX_MAPPED_REGIONS, "Max mapped regions out of bounds!");
	MappingRegion *region = lookup->region_storage + (lookup->region_storage_count++);
	{ // Setup region
		region->addr_old = addr_old;
		region->size_old = size_old;

		region->addr_new = addr_new;
		region->size_new = size_new;

		region->pointer_type = pointer_type;

#if DEBUG_RELOAD
		region->debug_tag = debug_tag;
#endif // DEBUG_RELOAD
	}

	// Add to the hash-map. This is a fast lookup for looking up pointers to objects.
	// Sometimes this isn't enough though. Pointers into arrays (or into other generic regions) can't use this map to find it's MappingRegion.
	// For this case, we also adds the region bounds into a bucket/linked list structure below
	{
		RegionHashEntry *map = lookup->region_hash;
		unsigned size = MAX_MAPPED_REGIONS;
		unsigned index = addr_old & size;
		RegionHashEntry *entry;
		for (unsigned i = 0; i < size; ++i) {
			entry = map + index;
			if (entry->key == 0) { // Look for an available slot
				break;
			}
			index++;
			if (index == size)
				index = 0;
		}
		ASSERT(entry->key == 0, "Couldn't find any available hash map entry!");
		entry->key = addr_old;
		entry->value = region;
	}

	{
		ASSERT(lookup->node_storage_count < MAX_ADDRESS_NODES, "Adress node storage out of bounds!");
		AddressNode *node = lookup->node_storage + (lookup->node_storage_count++);
		node->region = region;
		node->next  = 0;

		unsigned bucket_index_start = get_bucket_index(lookup, addr_old);
		unsigned bucket_index_end = get_bucket_index(lookup, addr_old + size_old);
		for (unsigned i = bucket_index_start; i <= bucket_index_end; ++i) {
			AddressNode *address_node = lookup->buckets[i];
			if (address_node == 0) {
				lookup->buckets[i] = node;
			} else {
				AddressNode *previous = address_node;
				bool found_same = false;
				while (address_node) {
					bool found_same = address_node->region->addr_old == addr_old && pointer_type == address_node->region->pointer_type; // A pointer have pointed here, fill in the correct values
					if (found_same) {
					 	address_node->region = node->region;
					 	break;
					}

					previous = address_node;
					address_node = address_node->next;
				}
				if (!found_same)
					previous->next = node;
			}
		}
	}
}