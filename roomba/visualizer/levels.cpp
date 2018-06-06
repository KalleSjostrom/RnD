#include "engine/levels.h"

Level make_level(Application &application) {
	Level l = {};

	Context c = {};
	c.model = PUSH_STRUCT(application.transient_arena, ModelCC);
	*c.model = load_model(application.transient_arena, DATA_FOLDER"/roomba.cobj");
	c.model->program_type = ProgramType_model;
	l.entity_data[l.count++] = make_entity_data(EntityType_Model, { 0, 0, 0 }, { 0.353, 0.353, 1 }, 0, c); // roomba.roomba_program_id

	return l;
}
