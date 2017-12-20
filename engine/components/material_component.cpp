struct RayMaterial {
	v3 reflection_color;
	v3 emittance_color;
	float roughness;
};

typedef RayMaterial MaterialCC;

struct MaterialComponent {
	RayMaterial materials[8];
	cid count;
};

void add(MaterialComponent &mc, Entity &entity, RayMaterial *material) {
	ASSERT((u32)mc.count < ARRAY_COUNT(mc.materials), "Component full!");
	entity.material_id = mc.count++;
	if (material) {
		mc.materials[entity.material_id] = *material;
	}
}
