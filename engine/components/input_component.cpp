struct Input {
	v3 move;
	b32 jump;
};

struct InputComponent {
	InputData *input_data;
	Input inputs[1];
	cid count;
};

void add(InputComponent &ic, Entity &entity) {
	ASSERT((u32)ic.count < ARRAY_COUNT(ic.inputs), "Component full!");
	entity.input_id = ic.count++;
}

void update(InputComponent &ic, float dt) {
	(void) dt;
	for (i32 i = 0; i < ic.count; ++i) {
		Input &input = ic.inputs[i];

		input.move.x = 0;

		if (IS_HELD(*ic.input_data, InputKey_D)) {
			input.move.x += 10;
		}
		if (IS_HELD(*ic.input_data, InputKey_A)) {
			input.move.x += -10;
		}
		input.jump = IS_PRESSED(*ic.input_data, InputKey_Space);
	}
}

v3 get_move(InputComponent &ic, Entity &entity) {
	return ic.inputs[entity.input_id].move;
}
b32 get_jump(InputComponent &ic, Entity &entity) {
	return ic.inputs[entity.input_id].jump;
}