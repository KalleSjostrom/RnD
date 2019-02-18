struct Type {
	void *data; // This is a lfXxx structure
};
struct TypeName {
	uint64_t id;
	size_t length;
	const char *name;
};
struct MemberList {
	Type *types;
	TypeName *names;
	unsigned *flags;
};
struct TypeTag {
	CV_typ_t type_index;
	unsigned tag;
};
struct TypeRecord {
	uint32_t flags;
	uint32_t type_index;
};

struct TypeContext {
	Type *types;
	TypeName *names;
	MemberList *member_lists;

	HashEntry *record_hashmap;
};

TypeName make_type_name(const char *name) {
	TypeName type_name = {};

	type_name.length = strlen(name);
	type_name.name = name;
	type_name.id = to_id64((unsigned)type_name.length, type_name.name);

	return type_name;
}
