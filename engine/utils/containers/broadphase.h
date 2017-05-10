struct Vector2 {
	float x, y;
};

struct BroadphaseEntry {
	Vector2 position;
	unsigned hash; // Hashed position. Represents the index in the broadphase array the entry would like to be stored.
	unsigned next; // The index to the next entry. If we have collisions, we stuff the colliding one on index != hash. To get to this index, we lookup hash and follow the next index.
};
struct Broadphase {
	BroadphaseEntry *entries;
	unsigned count;
	unsigned invalid_hash;
};

inline unsigned _calculate_hash(Broadphase &broadphase, int x, int y) {
	static int xprime = 492876863; /* arbitrary large prime */
	static int yprime = 633910099; /* arbitrary large prime */
	return ((x * xprime) ^ (y * yprime)) & (broadphase.count - 1);
}
inline unsigned _calculate_hash(Broadphase &broadphase, Vector2 &position) {
	int x = (int)floor(position.x);
	int y = (int)floor(position.y);
	return _calculate_hash(broadphase, x, y);
}
inline unsigned _get_empty_index(Broadphase &broadphase, unsigned hash) {
	ASSERT_MSG(hash != broadphase.invalid_hash, "Can't get index for invalid hash!");
	BroadphaseEntry *entry = broadphase.entries + hash;

	unsigned hash_mask = broadphase.count-1;

	for (unsigned offset = 0; offset < broadphase.count; offset++) {
		unsigned index = (hash + offset) & hash_mask;

		BroadphaseEntry *entry = broadphase.entries + index;
		if (entry->hash == broadphase.invalid_hash) {
			return index;
		}
	}
	return broadphase.invalid_hash;
}

void broadphase_init(Broadphase &broadphase, BroadphaseEntry *entries, unsigned count, unsigned invalid_hash) {
	broadphase.entries = entries;
	broadphase.count = count;
	broadphase.invalid_hash = invalid_hash;

	if (invalid_hash) { // Assumes that the entries memory is zeroed.
		for (unsigned i = 0; i < count; ++i) {
			entries[i].hash = invalid_hash;
			entries[i].next = invalid_hash;
		}
	}
}
unsigned broadphase_add(Broadphase &broadphase, Vector2 position/* EntityRef entity, InteractZone &interact_zone */) {
	unsigned hash = _calculate_hash(broadphase, position);
	ASSERT(hash != broadphase.invalid_hash);

	unsigned index = _get_empty_index(broadphase, hash);
	ASSERT_MSG(index != broadphase.invalid_hash, "Broadphase is full!");

	BroadphaseEntry *entry = broadphase.entries + hash; // Where we would like to be
	BroadphaseEntry *slot = broadphase.entries + index; // Where we got placed. These are different only if entry was occupied by someone else.

	slot->position = position;
	slot->hash = hash;

	if (hash != index) { // If there was something in our wanted place, we need to insert ourself in the linked list.
		// Insert slot in the linked list
 		ASSERT_MSG_VAR(entry->next != index, "Trying to insert a broadphase item in an already occupied slot! %u", index);

 		/*BroadphaseEntry *cursor = entry;
		// Find last link
		while (cursor->next) {
			cursor = broadphase.entries + cursor->next;
		}
		ASSERT(cursor->next == 0);
		cursor->next = index;*/

 		unsigned temp = entry->next;
		entry->next = index;
		slot->next = temp;
	}
	return index;
}

void broadphase_remove(Broadphase &broadphase, unsigned broadphase_index) {
	BroadphaseEntry *entry = broadphase.entries + broadphase_index;
	if (entry->hash == broadphase_index) { // We are placed at our optimal hash
		// We need to keep the linked list alive, just clear out our hash!
		entry->hash = broadphase.invalid_hash;
	} else {
		BroadphaseEntry *cursor = broadphase.entries + entry->hash; // Get the start of the linked list, which always start at our optimal hash.
		printf("next %u %u\n", cursor->next, entry->hash);
		// Look for our index
		while (cursor->next != broadphase_index && cursor->next != broadphase.invalid_hash) {
			cursor = broadphase.entries + cursor->next;
			printf("in -next %u\n", cursor->next);
		}
		ASSERT_MSG_VAR(cursor->next != broadphase.invalid_hash, "Reached end of list searching for broadphase_index to remove! (broadphase_index=%u)", broadphase_index);
		ASSERT_MSG_VAR(cursor->next == broadphase_index, "Could not find broadphase_index to remove! (broadphase_index=%u)", broadphase_index);

		// Cursor is now pointing to the index to remove. If the removed one have a next pointer, pass it to the one that pointed to us.
		cursor->next = entry->next;

		// Invalidate our entry
		entry->next = broadphase.invalid_hash;
		entry->hash = broadphase.invalid_hash;
	}
}

unsigned broadphase_move(Broadphase &broadphase, unsigned broadphase_index, Vector2 position) {
	BroadphaseEntry *entry = broadphase.entries + broadphase_index;
	ASSERT_MSG(entry->hash != broadphase.invalid_hash, "Trying to move an invalid interactable!");

	unsigned new_hash = _calculate_hash(broadphase, position);
	if (entry->hash == new_hash) { // Still in same cell
		entry->position = position;
		return broadphase_index;
	}

	// Switched cell, need to remove the old entry and add the new position.
	broadphase_remove(broadphase, broadphase_index);
	return broadphase_add(broadphase, position);
}

