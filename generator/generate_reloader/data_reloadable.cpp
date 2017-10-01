enum MemberFlag : u32 {
	MemberFlag_IsPointer = 1<<0, // If this is set but not MemberFlag_IsArray, then this points to some external memory that may be pointed to by someone else. We cannot copy any memory pointed to.
	MemberFlag_IsPointerPointer = 1<<1,
	MemberFlag_IsArray = 1<<2, // This implies is_pointer and that we have the count. It means we know that the pointer points to "inline" memory. We can copy this freely.
};
struct ReloadableMember {
	String name;
	String type;
	String full_type;

	u32 name_id;
	u32 type_id;
	u32 full_type_id;
	u32 union_id;

	u32 flags;
	i32 known_reloadable_type_index; // TRANSIENT - Index into the reloadable_array

	String full_type_underlined;

	// If MemberFlag_IsPointer and MemberFlag_IsArray is not set this will always be empty.
	// In other cases it might be empty implying a count of 1 or the max count of the array.
	String count;

	// If MemberFlag_IsArray, this is always non-empty and the string inbetween the brackets, [].
	String max_count;

	// Transient
	b32 is_chunk;
	b32 existed;
};
typedef ReloadableMember* ReloadableMemberArray;

void write_member(ReloadableMember &member, char **buffer) {
	write_string(member.name, buffer);
	write_string(member.type, buffer);
	write_string(member.full_type, buffer);

	write_u32(member.name_id, buffer);
	write_u32(member.type_id, buffer);
	write_u32(member.full_type_id, buffer);
	write_u32(member.union_id, buffer);

	write_u32(member.flags, buffer);

	write_string(member.full_type_underlined, buffer);

	write_string(member.count, buffer);
	write_string(member.max_count, buffer);
}
void read_member(MemoryArena &arena, ReloadableMember &member, char **buffer) {
	read_string(arena, member.name, buffer);
	read_string(arena, member.type, buffer);
	read_string(arena, member.full_type, buffer);

	read_u32(member.name_id, buffer);
	read_u32(member.type_id, buffer);
	read_u32(member.full_type_id, buffer);
	read_u32(member.union_id, buffer);

	read_u32(member.flags, buffer);

	read_string(arena, member.full_type_underlined, buffer);

	read_string(arena, member.count, buffer);
	read_string(arena, member.max_count, buffer);

	member.is_chunk = false;
	member.existed = false;
}

struct ReloadableStruct {
	String name;
	String full_name;
	String full_name_underlined;

	u32 name_id;
	u32 type_id;

	StringArray namespace_array;
	ReloadableMemberArray member_array;
	bool allocateable;

	// Transient data
	bool has_changed;
	bool is_chunk;
	bool is_visited;
	bool has_typeinfo;
	bool contains_pointers;
	bool __padding;
	bool __padding2;
};
struct ReloadableHashEntry {
	u32 key;
	i32 value;
};
HashMap_Make(ReloadableMap, ReloadableHashEntry);

typedef ReloadableStruct *ReloadableArray;

namespace reloadable {
	bool write(ReloadableArray &array, i32 count_snapshot, char **buffer) {
		i32 delta = array_count(array) - count_snapshot;
		write_i32(delta, buffer);

		bool changed = delta > 0;
		if (changed) {
			for (i32 i = count_snapshot; i < array_count(array); ++i) {
				ReloadableStruct &data = array[i];

				write_string(data.name, buffer);
				write_string(data.full_name, buffer);
				write_string(data.full_name_underlined, buffer);

				write_u32(data.type_id, buffer);
				write_u32(data.name_id, buffer);

				int namespace_count = array_count(data.namespace_array);
				write_i32(namespace_count, buffer);
				for (i32 j = 0; j < namespace_count; ++j) {
					write_string(data.namespace_array[j], buffer);
				}

				int member_count = array_count(data.member_array);
				write_i32(member_count, buffer);
				for (i32 j = 0; j < member_count; ++j) {
					write_member(data.member_array[j], buffer);
				}

				write_bool(data.allocateable, buffer);

				data.has_changed = true;
			}
		}
		return changed;
	}

	void read(MemoryArena &arena, ReloadableArray &array, char **buffer) {
		i32 delta;
		read_i32(delta, buffer);
		for (i32 i = 0; i < delta; ++i) {
			array_new_entry(array);
			ReloadableStruct &data = array_last(array);

			read_string(arena, data.name, buffer);
			read_string(arena, data.full_name, buffer);
			read_string(arena, data.full_name_underlined, buffer);

			read_u32(data.name_id, buffer);
			read_u32(data.type_id, buffer);

			read_serialized_array(data.namespace_array, buffer);
			for (i32 j = 0; j < array_count(data.namespace_array); ++j) {
				read_string(arena, data.namespace_array[j], buffer);
			}

			read_serialized_array(data.member_array, buffer);
			for (i32 j = 0; j < array_count(data.member_array); ++j) {
				read_member(arena, data.member_array[j], buffer);
			}

			read_bool(data.allocateable, buffer);

			data.has_changed = false;
		}
	}

	bool serialize(SerializationBuffer &sb, CacheHashEntry *entry, ReloadableArray &array, i32 count_snapshot) {
		char *buffer = sb.buffer + sb.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;
		bool changed = write(array, count_snapshot, &buffer);

		size_t size = (size_t)((intptr_t)buffer - start);
		sb.offset += size;
		entry->value.size = size;

		return changed;
	}

	void deserialize(MemoryArena &arena, CacheHashEntry *cache_entry, ReloadableArray &array) {
		char *buffer = (char*)cache_entry->value.buffer;
		read(arena, array, &buffer);
	}
}
