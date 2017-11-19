#include "engine/levels.h"

Level make_level() {
	Level l = {};

	Context c = {};
	l.entity_data[l.count++] = make_entity_data(EntityType_Block, { -1000, -600, 0 }, { 2000, 400, 1 }, 0, c);
	l.entity_data[l.count++] = make_entity_data(EntityType_BlockAvatar, { -512, 300, 0 }, { 20, 40, 1 }, 0, c);
	l.entity_data[l.count++] = make_entity_data(EntityType_Block, { 0, 0, 0 }, { 100, 100, 1 }, 0, c);
	l.entity_data[l.count++] = make_entity_data(EntityType_Sphere, { -100, 100, 0 }, { 20, 20, 1 }, 0, c);
	l.entity_data[l.count++] = make_entity_data(EntityType_Fluid, { 0, 0, 0 }, { 1, 1, 1 }, 0, c);

	return l;
};
