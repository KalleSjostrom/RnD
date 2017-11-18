struct Material {
	v3 reflection_color;
	v3 emittance_color;
	float roughness;
};

typedef Material MaterialCC;

struct MaterialComponent {
	Material instances[8];
	cid count;

	cid add(Material *material) {
		ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
		cid id = count++;
		if (material) {
			instances[id] = *material;
		}

		return id;
	}
};
