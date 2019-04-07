struct Input {
	Vector3 move;
	bool jump;
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
	for (int i = 0; i < ic.count; ++i) {
		Input &input = ic.inputs[i];

		input.move.x = 0;

		if (is_held(*ic.input_data, InputKey_D)) {
			input.move.x += 10;
		}
		if (is_held(*ic.input_data, InputKey_A)) {
			input.move.x += -10;
		}
		input.jump = is_pressed(*ic.input_data, InputKey_Space);
	}
}

Vector3 get_move(InputComponent &ic, Entity &entity) {
	return ic.inputs[entity.input_id].move;
}
bool get_jump(InputComponent &ic, Entity &entity) {
	return ic.inputs[entity.input_id].jump;
}