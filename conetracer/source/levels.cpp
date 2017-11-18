static Material materials[8] = {
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, 1 }, // Plane
	{ { 0.7f, 0.5f, 0.3f }, { 0, 0, 0 }, 0.3f },
	{ { 0.2f, 0.8f, 0.2f }, { 0, 0, 0 }, 0.15f },
};

Level make_level() {
	Level l = {};

	// Context c = {};
	// c.material = materials + 0;
	// l.entity_data[l.count++] = make_entity_data(EntityType_Block, { 1, 1, 0 }, { 1, 1, 1 }, 0, c);

	return l;
}
