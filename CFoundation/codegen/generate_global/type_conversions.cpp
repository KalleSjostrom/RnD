inline char *default_value_for_type(unsigned type_id) {
	if (type_id == float_id32) {
		return "0";
	} else if (type_id == double_id32) {
		return "0";
	} else if (type_id == bool_id32) {
		return "false";
	} else if (type_id == unsigned_id32) {
		return "0";
	} else if (type_id == int_id32) {
		return "0";
	} else if (type_id == long_id32) {
		return "0";
	} else if (type_id == Vector3_id32) {
		return "vector3(0, 0, 0)";
	} else if (type_id == Quaternion_id32) {
		return "quaternion_identity()";
	}
	return 0;
}

inline char *convert_flow_type(unsigned type_id) {
	if (type_id == float_id32) {
		return "float";
	} else if (type_id == bool_id32) {
		return "bool";
	} else if (type_id == UnitRef_id32) {
		return "unit";
	} else if (type_id == ActorPtr_id32) {
		return "actor";
	} else if (type_id == MoverPtr_id32) {
		return "mover";
	} else if (type_id == flow_string_id32) {
		return "string";
	} else if (type_id == Vector3_id32) {
		return "vector3";
	} else if (type_id == Quaternion_id32) {
		return "quaternion";
	} else if (type_id == event_id32) {
		return "event";
	}
	return 0;
}

inline char *field_size(unsigned type_id) {
	if (type_id == bool_id32) {
		return "4";
	} else if (type_id == int_id32) {
		return "4";
	} else if (type_id == float_id32) {
		return "4";
	} else if (type_id == unsigned_id32) {
		return "4";
	} else if (type_id == GameObjectId_id32) {
		return "4";
	} else if (type_id == Vector3_id32) {
		return "12";
	} else if (type_id == Quaternion_id32) {
		return "16";
	}
	return 0;
}

inline String convert_network_type(unsigned type_id) {
	if (type_id == int_id32) {
		return MAKE_STRING("int");
	} else if (type_id == unsigned_id32) {
		return MAKE_STRING("int");
	}
	return MAKE_STRING("int");
}