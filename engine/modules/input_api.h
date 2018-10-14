#pragma once

enum InputKey {
	InputKey_A = 0,
	InputKey_B,
	InputKey_C,
	InputKey_D,
	InputKey_E,
	InputKey_F,
	InputKey_G,
	InputKey_H,
	InputKey_I,
	InputKey_J,
	InputKey_K,
	InputKey_L,
	InputKey_M,
	InputKey_N,
	InputKey_O,
	InputKey_P,
	InputKey_Q,
	InputKey_R,
	InputKey_S,
	InputKey_T,
	InputKey_U,
	InputKey_V,
	InputKey_W,
	InputKey_X,
	InputKey_Y,
	InputKey_Z,

	InputKey_Space,
	InputKey_Enter,
	InputKey_Escape,
	InputKey_Tab,
	InputKey_Command,
	InputKey_Shift,
	InputKey_Option,
	InputKey_Control,
	InputKey_RightShift,
	InputKey_RightOption,
	InputKey_RightControl,
	InputKey_Function,

	InputKey_MouseLeft,
	InputKey_MouseMiddle,
	InputKey_MouseRight,
	InputKey_MouseX1,
	InputKey_MouseX2,

	InputKey_Count
};

struct InputTime {
	float pressed;
	float released;
};
struct InputData {
	bool pressed[InputKey_Count];
	bool released[InputKey_Count];
	int mouse_x; int mouse_y;
	int mouse_xrel; int mouse_yrel;
	InputTime times[InputKey_Count];
};
__forceinline bool is_held(InputData &input, InputKey key) {
	return input.times[key].pressed > input.times[key].released;
}
__forceinline bool is_pressed(InputData &input, InputKey key) {
	return input.pressed[key];
}
