TypeRecordPair get_type_record_pair(TypeContextPair *type_context, TypeName *name) {
	TypeRecordPair type_record = {};

	type_record.o = get_type_record(type_context->o, name->id);
	type_record.n = get_type_record(type_context->n, name->id);

	return type_record;
}

OffsetPair get_size_pair(TypeContextPair *type_context, TypeRecordPair *type_record) {
	OffsetPair size = {};

	size.o = get_size(type_context->o, type_record->o->type_index);
	size.n = get_size(type_context->n, type_record->n->type_index);

	return size;
}

MemberListPair get_member_list_pair(TypeContextPair *type_context, TypeRecordPair *type_record) {
	MemberListPair member_list = {};

	member_list.o = get_member_list(type_context->o, type_record->o);
	member_list.n = get_member_list(type_context->n, type_record->n);

	return member_list;
}
