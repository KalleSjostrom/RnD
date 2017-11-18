struct EntityData {
	EntityType type;
	v3 offset;
	v3 size;
	float rotation;
	Context context;
};
EntityData make_entity_data(EntityType type, v3 offset, v3 size, float rotation, Context &context) {
	EntityData data = {};
	data.type = type;
	data.offset = offset;
	data.size = size;
	data.rotation = rotation;
	data.context = context;
	return data;
};

struct Level {
	i32 count;
	EntityData entity_data[512];
};

Level make_level();
