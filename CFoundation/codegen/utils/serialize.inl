inline void write_bool(bool value, char **buffer) {
	*(bool*)(*buffer) = value;
	*buffer += sizeof(bool);
}
inline void write_unsigned(unsigned value, char **buffer) {
	*(unsigned*)(*buffer) = value;
	*buffer += sizeof(unsigned);
}
inline void write_u64(uint64_t value, char **buffer) {
	*(uint64_t*)(*buffer) = value;
	*buffer += sizeof(uint64_t);
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
inline void read_unsigned(unsigned &value, char **buffer) {
	value = *(unsigned*)(*buffer);
	*buffer += sizeof(unsigned);
}
inline void read_u64(uint64_t &value, char **buffer) {
	value = *(uint64_t*)(*buffer);
	*buffer += sizeof(uint64_t);
}
inline void read_string(MemoryArena &arena, String &string, char **buffer) {
	read_unsigned(string.length, buffer);
	char *memory = allocate_memory(arena, string.length + 1);
	memcpy(memory, *buffer, string.length + 1);
	string.text = memory;
	*buffer += (string.length + 1) * sizeof(char);
}
