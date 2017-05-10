inline void write_bool(bool value, char **buffer) {
	*(bool*)(*buffer) = value;
	*buffer += sizeof(bool);
}
inline void write_unsigned(unsigned value, char **buffer) {
	*(unsigned*)(*buffer) = value;
	*buffer += sizeof(unsigned);
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
	write_unsigned(string.length, buffer);
	memcpy(*buffer, string.text, string.length);
	(*buffer)[string.length] = '\0';
	*buffer += (string.length + 1) * sizeof(char);
}

inline void read_bool(bool &value, char **buffer) {
	value = *(bool*)(*buffer);
	*buffer += sizeof(bool);
}
inline void read_unsigned(u32 &value, char **buffer) {
	value = *(u32*)(*buffer);
	*buffer += sizeof(u32);
}
inline void read_u64(u64 &value, char **buffer) {
	value = *(u64*)(*buffer);
	*buffer += sizeof(u64);
}
inline void read_string(MemoryArena &arena, String &string, char **buffer) {
	read_unsigned(string.length, buffer);
	char *memory = allocate_memory(arena, string.length + 1);
	memcpy(memory, *buffer, string.length + 1);
	string.text = memory;
	*buffer += (string.length + 1) * sizeof(char);
}
