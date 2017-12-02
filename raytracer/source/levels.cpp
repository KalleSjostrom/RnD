#include "engine/levels.h"

static RayMaterial materials[8] = {
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, 1 }, // Plane
	{ { 0.7f, 0.5f, 0.3f }, { 0, 0, 0 }, 0.3f },
	{ { 0.2f, 0.8f, 0.2f }, { 0, 0, 0 }, 0.15f },
};

Level make_level() {
	Level l = {};

	Context c = {};
	c.material = materials + 0;
	l.entity_data[l.count++] = make_entity_data(EntityType_Plane, { -400, -100, -200 }, { 800, 0, 400 }, 0, c);

	c.material = materials + 1;
	l.entity_data[l.count++] = make_entity_data(EntityType_Sphere, { 0, -100, 0 }, { 50, 50, 50 }, 0, c);

	c.material = materials + 2;
	l.entity_data[l.count++] = make_entity_data(EntityType_Sphere, { 200, -50, 0 }, { 50, 50, 50 }, 0, c);

	// c.material = materials + 3;
	// l.entity_data[l.count++] = make_entity_data(EntityType_Sphere, { 200, -50, 0 }, { 50, 50, 50 }, 0, c);

	// c.material = materials + 4;
	// l.entity_data[l.count++] = make_entity_data(EntityType_Sphere, { 200, -50, 0 }, { 50, 50, 50 }, 0, c);

	return l;
}
