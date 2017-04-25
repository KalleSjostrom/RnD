InputKey get_input_key_for(int key) {
	switch (key) {
		case 0x00: { return InputKey_Left; }; // KEY_A
		case 0x01: { return InputKey_Down; }; // KEY_S
		case 0x02: { return InputKey_Right; }; // KEY_D
		case 0x0D: { return InputKey_Up; }; // KEY_W

		case 0x24: { return InputKey_Action; }; // KEY_ENTER
		case 0x31: { return InputKey_Jump; }; // KEY_SPACE

		default: { return InputKey_Count; };
	};
}

static Input _input = {};

void key_down(int key, int modifier_flags) {
	(void) modifier_flags;
	InputKey input_key = get_input_key_for(key);
	if (input_key != InputKey_Count) {
		_input.pressed[input_key] = true;
		_input.times[input_key].pressed = ENGINE_TIME;
	}
}
void key_up(int key, int modifier_flags) {
	(void) modifier_flags;
	InputKey input_key = get_input_key_for(key);
	if (input_key != InputKey_Count) {
		_input.released[input_key] = true;
		_input.times[input_key].released = ENGINE_TIME;
	}
}
