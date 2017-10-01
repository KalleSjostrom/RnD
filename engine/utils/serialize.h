inline void write_bool(bool value, char **buffer) {
	*(bool*)(*buffer) = value;
	*buffer += sizeof(bool);
}
inline void write_u32(u32 value, char **buffer) {
	*(u32*)(*buffer) = value;
	*buffer += sizeof(u32);
}
inline void write_i32(i32 value, char **buffer) {
	*(i32*)(*buffer) = value;
	*buffer += sizeof(i32);
}
inline void write_u64(u64 value, char **buffer) {
	*(u64*)(*buffer) = value;
	*buffer += sizeof(u64);
}
inline void write_f32(f32 value, char **buffer) {
	*(f32*)(*buffer) = value;
	*buffer += sizeof(f32);
}
inline void write_f64(f64 value, char **buffer) {
	*(f64*)(*buffer) = value;
	*buffer += sizeof(f64);
}
inline void write_string(String &string, char **buffer) {
	write_i32(string.length, buffer);
	memcpy(*buffer, string.text, (size_t)string.length);
	(*buffer)[string.length] = '\0';
	*buffer += (u32)(string.length + 1) * sizeof(char);
}

inline void read_bool(bool &value, char **buffer) {
	value = *(bool*)(*buffer);
	*buffer += sizeof(bool);
}
inline void read_u32(u32 &value, char **buffer) {
	value = *(u32*)(*buffer);
	*buffer += sizeof(u32);
}
inline void read_i32(i32 &value, char **buffer) {
	value = *(i32*)(*buffer);
	*buffer += sizeof(i32);
}
inline void read_u64(u64 &value, char **buffer) {
	value = *(u64*)(*buffer);
	*buffer += sizeof(u64);
}
inline void read_string(MemoryArena &arena, String &string, char **buffer) {
	read_i32(string.length, buffer);
	size_t size = (size_t)(string.length + 1);
	char *memory = PUSH_STRING(arena, size);
	memcpy(memory, *buffer, size);
	string.text = memory;
	*buffer += size * sizeof(char);
}
#define read_serialized_array(array, buffer) do { \
	int array_count; \
	read_i32(array_count, buffer); \
	array_init(array, array_count < 4 ? 4 : array_count); \
	array_set_count(array, array_count); \
} while(0)
