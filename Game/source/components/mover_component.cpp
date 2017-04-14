namespace mover_component {
	struct Instance {
		v3 position;
		v3 velocity;
		v3 acceleration;
	};

	struct MoverComponent {
		int count;
		Instance instances[8];

		int add(v3 position) {
			ASSERT(count < ARRAY_COUNT(instances), "Component full!");
			int id = count++;
			Instance &instance = instances[id];

			instance.position = position;

			return id;
		}

		inline void add_acceleration(int id, v3 acceleration) {
			Instance &instance = instances[id];
			instance.acceleration += acceleration;
		}
		inline void set_velocity(int id, v3 velocity) {
			Instance &instance = instances[id];
			instance.velocity = velocity;
		}
		inline v3 get_position(int id) {
			return instances[id].position;
		}

		void update(float dt) {
			for (int i = 0; i < count; ++i) {
				Instance &instance = instances[i];
				instance.velocity += (instance.acceleration - instance.velocity * 10.f) * dt;
				instance.position += instance.velocity * dt;

				// instance.velocity *= 0.8f;
				instance.acceleration = V3(0, 0, 0);
			}
		}
	};
}
