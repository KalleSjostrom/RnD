enum OnAddedBehavior {
	ZERO,
	CONSTRUCT,
};

namespace reloadable {
	struct Member {
		String name;
		unsigned name_id;

		String type;
		unsigned type_id;

		String full_type;
		unsigned full_type_id;

		String full_type_underlined;

		bool is_pointer; // If this is true but not is_array, then this points to some external memory that may be pointed to by someone else. We cannot copy any memory pointed to.
		bool is_pointer_pointer;
		bool is_array; // This implies is_pointer and that we have the count. It means we know that the pointer points to "inline" memory. We can copy this freely.

		// If is_pointer and is_array is false this will always be empty.
		// In other cases it might be empty implying a count of 1 or the max count of the array.
		String count;

		// If is_array, this is always non-empty and the string inbetween the brackets, [].
		String max_count;
	};
	void write_member(Member &member, char **buffer) {
		write_string(member.name, buffer);
		write_unsigned(member.name_id, buffer);

		write_string(member.type, buffer);
		write_unsigned(member.type_id, buffer);

		write_string(member.full_type, buffer);
		write_unsigned(member.full_type_id, buffer);

		write_string(member.full_type_underlined, buffer);

		write_bool(member.is_pointer, buffer);
		write_bool(member.is_pointer_pointer, buffer);
		write_bool(member.is_array, buffer);

		write_string(member.count, buffer);
		write_string(member.max_count, buffer);
	}
	void read_member(MemoryArena &arena, Member &member, char **buffer) {
		read_string(arena, member.name, buffer);
		read_unsigned(member.name_id, buffer);

		read_string(arena, member.type, buffer);
		read_unsigned(member.type_id, buffer);

		read_string(arena, member.full_type, buffer);
		read_unsigned(member.full_type_id, buffer);

		read_string(arena, member.full_type_underlined, buffer);

		read_bool(member.is_pointer, buffer);
		read_bool(member.is_pointer_pointer, buffer);
		read_bool(member.is_array, buffer);

		read_string(arena, member.count, buffer);
		read_string(arena, member.max_count, buffer);
	}

	struct Data {
		String full_name;
		String full_name_underlined;

		unsigned namespace_count;
		String namespaces[8];

		String name;
		unsigned name_id;

		OnAddedBehavior added_behavior;

		unsigned member_count;
		Member members[128];
	};
	#include "../utils/data_generic.inl"

	void write(unsigned count_snapshot, Array &array, char **buffer) {
		unsigned delta;

		delta = array.count - count_snapshot;
		write_unsigned(delta, buffer);
		if (delta > 0) {
			array.changed = true;
			for (int i = count_snapshot; i < array.count; ++i) {
				Data &data = array.entries[i];

				write_string(data.full_name, buffer);
				write_string(data.full_name_underlined, buffer);

				write_unsigned(data.namespace_count, buffer);
				for (int j = 0; j < data.namespace_count; ++j) {
					write_string(data.namespaces[j], buffer);
				}

				write_string(data.name, buffer);
				write_unsigned(data.name_id, buffer);

				write_unsigned(*(unsigned*)&data.added_behavior, buffer);

				write_unsigned(data.member_count, buffer);
				for (int j = 0; j < data.member_count; ++j) {
					write_member(data.members[j], buffer);
				}
			}
		}
	}

	void read(MemoryArena &arena, Array &array, char **buffer) {
		unsigned num_entries;

		read_unsigned(num_entries, buffer);
		for (unsigned i = 0; i < num_entries; ++i) {
			Data &data = array.entries[array.count++];

			read_string(arena, data.full_name, buffer);
			read_string(arena, data.full_name_underlined, buffer);

			read_unsigned(data.namespace_count, buffer);
			for (unsigned j = 0; j < data.namespace_count; ++j) {
				read_string(arena, data.namespaces[j], buffer);
			}

			read_string(arena, data.name, buffer);
			read_unsigned(data.name_id, buffer);

			read_unsigned(*(unsigned*)&data.added_behavior, buffer);

			read_unsigned(data.member_count, buffer);
			for (unsigned j = 0; j < data.member_count; ++j) {
				read_member(arena, data.members[j], buffer);
			}
		}
	}

	void serialize(MemoryArena &arena, unsigned count_snapshot, CacheHashEntry *entry, Array &array) {
		char *buffer = arena.memory + arena.offset;
		entry->value.buffer = buffer;
		intptr_t start = (intptr_t)buffer;

		write(count_snapshot, array, &buffer);
		
		size_t size = (size_t)((intptr_t)buffer - start);
		arena.offset += size;
		entry->value.size = size;
	}

	void deserialize(MemoryArena &arena, char *filepath, CacheHashEntry *entry, Array &array) {
		char *buffer = (char*)entry->value.buffer;

		read(arena, array, &buffer);
	}
}

typedef reloadable::Data ReloadableStruct;
typedef reloadable::Member ReloadableMember;
typedef reloadable::Array ReloadableArray;
