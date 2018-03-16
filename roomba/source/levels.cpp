#include "engine/levels.h"

Level make_level() {
	Level l = {};

	Context c = {};
	l.entity_data[l.count++] = make_entity_data(EntityType_Roomba, { 0, 0, 0 }, { 100, 100, 100 }, 0, c); // roomba.roomba_program_id
	l.entity_data[l.count++] = make_entity_data(EntityType_Ruler, { 0, 0, 0 }, { 100, 100, 100 }, 0, c); // roomba.line_program_id

	return l;
}
