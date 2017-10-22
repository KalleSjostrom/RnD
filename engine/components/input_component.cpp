namespace input_component {
	struct Instance {
		v3 move;
		b32 jump;
	};

	struct InputComponent {
		Input *input;

		Instance instances[1];
		i32 count;
		i32 __padding;

		void set_input(Input *i) {
			input = i;
		}

		i32 add() {
			ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
			i32 id = count++;
			Instance &instance = instances[id];

			return id;
		}

		void update(float dt) {
			for (i32 i = 0; i < count; ++i) {
				Instance &instance = instances[i];

				instance.move.x = 0;

				if (IS_HELD(*input, InputKey_D)) {
					instance.move.x += 10;
				}
				if (IS_HELD(*input, InputKey_A)) {
					instance.move.x += -10;
				}
				instance.jump = IS_PRESSED(*input, InputKey_Space);
			}
		}
	};
}
