struct Input {
	v3 move;
	b32 jump;
};

struct InputComponent {
	InputData *input_data;
	Input inputs[1];
	cid count;

	void set_input_data(InputData *i) {
		input_data = i;
	}

	cid add() {
		ASSERT((u32)count < ARRAY_COUNT(inputs), "Component full!");
		cid id = count++;
		return id;
	}

	void update(float dt) {
		(void) dt;
		for (i32 i = 0; i < count; ++i) {
			Input &input = inputs[i];

			input.move.x = 0;

			if (IS_HELD(*input_data, InputKey_D)) {
				input.move.x += 10;
			}
			if (IS_HELD(*input_data, InputKey_A)) {
				input.move.x += -10;
			}
			input.jump = IS_PRESSED(*input_data, InputKey_Space);
		}
	}
};
