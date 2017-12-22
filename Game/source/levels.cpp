#include "engine/levels.h"
#include "engine/utils/obj_reader.cpp"

#define DATA_FOLDER "../../game/out/data/"

ModelCC load_model(MemoryArena &arena, const char *path) {
	MeshData mesh_data = read_obj(arena, path);
	ModelCC model_cc = {};
	model_cc.mesh_data = mesh_data;

	model_cc.buffer_type = GL_STATIC_DRAW;
	model_cc.draw_mode = GL_TRIANGLES;

	model_cc.program_type = ProgramType_model;

	return model_cc;
}

Level make_level(Application &application) {
	Level l = {};

// model = {
// 	program = ProgramType_default
// 	obj_path = "assets/quad.obj"
// }
// actor = {
// 	shape = ShapeType_Box
// }

	// EntityAsset &entity = load_entity(DATA_FOLDER"block.centity");

	{
		Context c = {};
		ModelCC model_cc = load_model(application.transient_arena, DATA_FOLDER"quad.cobj");
		c.model = &model_cc;
		c.model->program_type = ProgramType_default;
		l.entity_data[l.count++] = make_entity_data(EntityType_Model, { -1000, -600, 0 }, { 2000, 400, 1 }, 0, c);
		l.entity_data[l.count++] = make_entity_data(EntityType_Model, { -300, 0, 0 }, { 100, 100, 1 }, 0, c);
	}

	// l.entity_data[l.count++] = make_entity_data(EntityType_Block, { -1000, -600, 0 }, { 2000, 400, 1 }, 0, c);
	// l.entity_data[l.count++] = make_entity_data(EntityType_BlockAvatar, { -500, 300, 0 }, { 20, 40, 1 }, 0, c);
	// l.entity_data[l.count++] = make_entity_data(EntityType_Block, { 0, 0, 0 }, { 100, 100, 1 }, 0, c);
	// l.entity_data[l.count++] = make_entity_data(EntityType_Sphere, { -100, 100, 0 }, { 20, 20, 1 }, 0, c);

	return l;
};
