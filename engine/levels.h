#include "engine/utils/obj_reader.cpp"

struct EntityData {
	EntityType type;
	Vector3 offset;
	Vector3 size;
	float rotation;
	Context context;
};
EntityData make_entity_data(EntityType type, Vector3 offset, Vector3 size, float rotation, Context &context) {
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

Level make_level(Application &application);

ModelCC load_model(ArenaAllocator &arena, const char *path) {
	MeshData mesh_data = read_obj(arena, path);
	ModelCC model_cc = {};
	model_cc.mesh_data = mesh_data;

	model_cc.buffer_type = GL_STATIC_DRAW;
	model_cc.draw_mode = GL_TRIANGLES;

	model_cc.program_type = ProgramType_model;

	return model_cc;
}
