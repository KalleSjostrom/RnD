#pragma once

struct MemberListPair {
	struct MemberList *n;
	struct MemberList *o;
};
struct TypeRecordPair {
	struct TypeRecord *n;
	struct TypeRecord *o;
};
struct TypeContextPair {
	struct TypeContext *n;
	struct TypeContext *o;
};

struct OffsetPair {
	size_t n;
	size_t o;
};
struct AddressPair {
	intptr_t n;
	intptr_t o;
};

__forceinline OffsetPair &operator+=(OffsetPair &a, OffsetPair &b) {
	a.n += b.n;
	a.o += b.o;
	return a;
}

__forceinline AddressPair operator+(AddressPair address, OffsetPair offset) {
	AddressPair ap;

	ap.n = address.n + offset.n;
	ap.o = address.o + offset.o;

	return ap;
}
__forceinline AddressPair &operator+=(AddressPair &a, OffsetPair &b) {
	a = a + b;
	return a;
}

TypeRecordPair get_type_record_pair(struct TypeContextPair *type_context, struct TypeName *name);
OffsetPair get_size_pair(struct TypeContextPair *type_context, struct TypeRecordPair *type_record);
MemberListPair get_member_list_pair(struct TypeContextPair *type_context, struct TypeRecordPair *type_record);
