struct Mover {
	v3 position;
	v3 wanted_translation;
	v3 velocity;
	v3 acceleration;
};

struct MoverComponent {
	Mover movers[8];
	cid count;
};

void add(MoverComponent &mc, Entity &entity, v3 position) {
	ASSERT((u32)mc.count < ARRAY_COUNT(mc.movers), "Component full!");
	entity.mover_id = mc.count++;
	Mover &mover = mc.movers[entity.mover_id];

	mover.position = position;
}

v3 &get_position(MoverComponent &mc, Entity &entity) {
	return mc.movers[entity.mover_id].position;
}
v3 &get_wanted_translation(MoverComponent &mc, Entity &entity) {
	return mc.movers[entity.mover_id].wanted_translation;
}
v3 &get_velocity(MoverComponent &mc, Entity &entity) {
	return mc.movers[entity.mover_id].velocity;
}
void set_velocity(MoverComponent &mc, Entity &entity, v3 &velocity) {
	mc.movers[entity.mover_id].velocity = velocity;
}
inline void add_acceleration(MoverComponent &mc, Entity &entity, v3 &acceleration) {
	Mover &mover = mc.movers[entity.mover_id];
	mover.acceleration += acceleration;
}
inline void add_impulse(MoverComponent &mc, Entity &entity, v3 &impulse) {
	Mover &mover = mc.movers[entity.mover_id];
	mover.velocity += impulse;
}

void update(MoverComponent &mc, float dt) {
	for (i32 i = 0; i < mc.count; ++i) {
		Mover &mover = mc.movers[i];
		float g = -9.82f * 40;
		v3 acceleration = mover.acceleration + V3(0, g, 0);
		mover.velocity += acceleration * dt;
		mover.wanted_translation = mover.velocity * dt;
		// mover.position += mover.velocity * dt;

		// mover.velocity *= 0.8f;
		mover.acceleration = V3(0, 0, 0);
	}
}
