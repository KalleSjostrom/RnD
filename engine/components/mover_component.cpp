struct Mover {
	v3 position;
	v3 wanted_translation;
	v3 velocity;
	v3 acceleration;
};

struct MoverComponent {
	Mover movers[8];
	cid count;

	cid add(v3 position) {
		ASSERT((u32)count < ARRAY_COUNT(movers), "Component full!");
		cid id = count++;
		Mover &mover = movers[id];

		mover.position = position;

		return id;
	}

	inline void add_acceleration(i32 id, v3 &acceleration) {
		Mover &mover = movers[id];
		mover.acceleration += acceleration;
	}
	inline void add_impulse(i32 id, v3 &impulse) {
		Mover &mover = movers[id];
		mover.velocity += impulse;
		printf("%g\n", (double)mover.velocity.y);
	}

	void update(float dt) {
		for (i32 i = 0; i < count; ++i) {
			Mover &mover = movers[i];
			float g = -9.82f * 40;
			v3 acceleration = mover.acceleration + V3(0, g, 0);
			mover.velocity += acceleration * dt;
			mover.wanted_translation = mover.velocity * dt;
			// mover.position += mover.velocity * dt;

			// mover.velocity *= 0.8f;
			mover.acceleration = V3(0, 0, 0);
		}
	}
};
