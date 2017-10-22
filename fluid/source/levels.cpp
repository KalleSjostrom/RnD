struct EntityData {
	EntityType type;
	float x;
	float y;
	float w;
	float h;
	float rotation;
};

struct Level {
	i32 count;
	EntityData entity_data[512];
};

static Level level = {
	5, {
		{ EntityType_Block, -1000, -600, 2000, 400, 0 },
		{ EntityType_BlockAvatar, -512, 300, 20, 40, 0 },

		{ EntityType_Block, 0, 0, 100, 100, 0 },
		{ EntityType_Sphere, 0, 0, 20, 20, 0 },
		{ EntityType_Fluid, 0, 0, 0, 0, 0 },
	},
};
