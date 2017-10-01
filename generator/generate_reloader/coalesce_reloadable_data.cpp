void visit_struct_chunk(ReloadableStruct &s, ReloadableStruct **struct_collection, ReloadableMap &reloadable_map) {
	s.is_visited = true;

	if (s.has_changed) {
		s.is_chunk = false; // Treat changed structs as non-chunks.
		return;
	}

	bool is_chunk = true;
	for (i32 i = 0; i < array_count(s.member_array); ++i) {
		ReloadableMember &m = s.member_array[i];
		if (m.known_reloadable_type_index >= 0) {
			ReloadableStruct &child = *struct_collection[m.known_reloadable_type_index];
			if (!child.is_visited) { // I have _not_ checked this struct before
				visit_struct_chunk(child, struct_collection, reloadable_map); // So visit it!
			}

			m.is_chunk = child.is_chunk; // Mark this member as is_chunk if it's type is chunk
		} else { // I don't know what this struct is, treat it as a chunk
			m.is_chunk = true;
		}

		is_chunk = is_chunk && m.is_chunk; // I'm only a chunk if all my children are.
	}
	s.is_chunk = is_chunk && !s.contains_pointers; // Can't treat stuff contains pointers as chunks atm, since it will cause the pointer value to also get copied.
}

void visit_struct_ptr(ReloadableStruct &s, ReloadableStruct **struct_collection, ReloadableMap &reloadable_map) {
	s.is_visited = true;

	bool contains_pointers = false;
	for (i32 i = 0; i < array_count(s.member_array) && !contains_pointers; ++i) {
		ReloadableMember &m = s.member_array[i];
		b32 is_pointer = m.flags & MemberFlag_IsPointer;
		b32 is_pointer_pointer = m.flags & MemberFlag_IsPointerPointer;
		b32 is_array = m.flags & MemberFlag_IsArray;
		if (is_pointer && (!is_array || is_pointer_pointer)) {
			contains_pointers = true;
		} else if (m.known_reloadable_type_index >= 0) {
			ReloadableStruct &child = *struct_collection[m.known_reloadable_type_index];
			if (!child.is_visited) { // I have _not_ checked this struct before
				visit_struct_ptr(child, struct_collection, reloadable_map); // So visit it!
			}

			contains_pointers = child.contains_pointers; // Mark this member as is_chunk if it's type is chunk
		}
	}
	s.contains_pointers = contains_pointers;
}

void coalesce_reloadable_data(ReloadableStruct **struct_collection, i32 struct_collection_count, ReloadableMap &reloadable_map) {
	for (i32 i = 0; i < struct_collection_count; ++i) {
		ReloadableStruct &s = *struct_collection[i];
		s.is_visited = false;
		for (i32 j = 0; j < array_count(s.member_array); ++j) {
			ReloadableMember &m = s.member_array[j];
			// m.existed = true;
			HashMap_Lookup(entry, reloadable_map, m.full_type_id, 0);
			if (entry->key == m.full_type_id) {
				m.known_reloadable_type_index = entry->value;
			} else {
				m.known_reloadable_type_index = -1;
			}
		}
	}

	for (i32 i = 0; i < struct_collection_count; ++i) {
		ReloadableStruct &s = *struct_collection[i];
		if (!s.is_visited) {
			visit_struct_ptr(s, struct_collection, reloadable_map);
		}
	}

	for (i32 i = 0; i < struct_collection_count; ++i) {
		ReloadableStruct &s = *struct_collection[i];
		s.is_visited = false;
	}

	for (i32 i = 0; i < struct_collection_count; ++i) {
		ReloadableStruct &s = *struct_collection[i];
		if (!s.is_visited) {
			visit_struct_chunk(s, struct_collection, reloadable_map);
		}
	}
}
