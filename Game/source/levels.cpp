#include "engine/levels.h"

#define DATA_FOLDER "../../game/out/data"

Level make_level(Application &application) {
	Level l = {};

// model = {
// 	program = ProgramType_default
// 	obj_path = "assets/quad.obj"
// }
// actor = {
// 	shape = ShapeType_Box
// }

	// EntityAsset &entity = load_entity(DATA_FOLDER"/block.centity");

	{
		Context c = {};
		c.model = PUSH_STRUCT(application.transient_arena, ModelCC);
		*c.model = load_model(application.transient_arena, DATA_FOLDER"/quad.cobj");
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
