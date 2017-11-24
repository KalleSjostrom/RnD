struct RayMaterial {
	v3 reflection_color;
	v3 emittance_color;
	float roughness;
};

typedef RayMaterial MaterialCC;

struct MaterialComponent {
	RayMaterial instances[8];
	cid count;

	cid add(RayMaterial *material) {
		ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
		cid id = count++;
		if (material) {
			instances[id] = *material;
		}

		return id;
	}
};
