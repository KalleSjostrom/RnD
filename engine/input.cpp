InputKey get_keyboard_key_for(int key) {
	switch (key) {
#if 0
		case  0: { return InputKey_A; }; break;
		case 11: { return InputKey_B; }; break;
		case  8: { return InputKey_C; }; break;
		case  2: { return InputKey_D; }; break;
		case 14: { return InputKey_E; }; break;
		case  3: { return InputKey_F; }; break;
		case  5: { return InputKey_G; }; break;
		case  4: { return InputKey_H; }; break;
		case 34: { return InputKey_I; }; break;
		case 38: { return InputKey_J; }; break;
		case 40: { return InputKey_K; }; break;
		case 37: { return InputKey_L; }; break;
		case 46: { return InputKey_M; }; break;
		case 45: { return InputKey_N; }; break;
		case 31: { return InputKey_O; }; break;
		case 35: { return InputKey_P; }; break;
		case 12: { return InputKey_Q; }; break;
		case 15: { return InputKey_R; }; break;
		case  1: { return InputKey_S; }; break;
		case 17: { return InputKey_T; }; break;
		case 32: { return InputKey_U; }; break;
		case  9: { return InputKey_V; }; break;
		case 13: { return InputKey_W; }; break;
		case  7: { return InputKey_X; }; break;
		case 16: { return InputKey_Y; }; break;
		case  6: { return InputKey_Z; }; break;

		case 49: { return InputKey_Space; }; break;
		case 36: { return InputKey_Enter; }; break;
		case 53: { return InputKey_Escape; }; break;
		case 48: { return InputKey_Tab; }; break;
		case 55: { return InputKey_Command; } break;
		case 56: { return InputKey_Shift; } break;
		case 58: { return InputKey_Option; } break;
		case 59: { return InputKey_Control; } break;
		case 60: { return InputKey_RightShift; } break;
		case 61: { return InputKey_RightOption; } break;
		case 62: { return InputKey_RightControl; } break;
		case 63: { return InputKey_Function; } break;
#else
		case SDLK_a: { return InputKey_A; }; break;
		case SDLK_b: { return InputKey_B; }; break;
		case SDLK_c: { return InputKey_C; }; break;
		case SDLK_d: { return InputKey_D; }; break;
		case SDLK_e: { return InputKey_E; }; break;
		case SDLK_f: { return InputKey_F; }; break;
		case SDLK_g: { return InputKey_G; }; break;
		case SDLK_h: { return InputKey_H; }; break;
		case SDLK_i: { return InputKey_I; }; break;
		case SDLK_j: { return InputKey_J; }; break;
		case SDLK_k: { return InputKey_K; }; break;
		case SDLK_l: { return InputKey_L; }; break;
		case SDLK_m: { return InputKey_M; }; break;
		case SDLK_n: { return InputKey_N; }; break;
		case SDLK_o: { return InputKey_O; }; break;
		case SDLK_p: { return InputKey_P; }; break;
		case SDLK_q: { return InputKey_Q; }; break;
		case SDLK_r: { return InputKey_R; }; break;
		case SDLK_s: { return InputKey_S; }; break;
		case SDLK_t: { return InputKey_T; }; break;
		case SDLK_u: { return InputKey_U; }; break;
		case SDLK_v: { return InputKey_V; }; break;
		case SDLK_w: { return InputKey_W; }; break;
		case SDLK_x: { return InputKey_X; }; break;
		case SDLK_y: { return InputKey_Y; }; break;
		case SDLK_z: { return InputKey_Z; }; break;

		case SDLK_SPACE: { return InputKey_Space; }; break;
		case SDLK_RETURN: { return InputKey_Enter; }; break;
		case SDLK_ESCAPE: { return InputKey_Escape; }; break;
		case SDLK_TAB: { return InputKey_Tab; }; break;
		// case 55: { return InputKey_Command; } break;
		// case 56: { return InputKey_Shift; } break;
		// case 58: { return InputKey_Option; } break;
		// case 59: { return InputKey_Control; } break;
		// case 60: { return InputKey_RightShift; } break;
		// case 61: { return InputKey_RightOption; } break;
		// case 62: { return InputKey_RightControl; } break;
		// case 63: { return InputKey_Function; } break;
#endif
		default: { return InputKey_Count; }; break;
	};
}

InputKey get_mouse_key_for(int key) {
	switch (key) {
		case SDL_BUTTON_LEFT: { return InputKey_MouseLeft; } break;
		case SDL_BUTTON_MIDDLE: { return InputKey_MouseMiddle; } break;
		case SDL_BUTTON_RIGHT: { return InputKey_MouseRight; } break;
		case SDL_BUTTON_X1: { return InputKey_MouseX1; } break;
		case SDL_BUTTON_X2: { return InputKey_MouseX2; } break;

		default: { return InputKey_Count; }; break;
	}
}

static InputData _input = {};

void key_down(int key, int modifier_flags) {
	(void) modifier_flags;
	InputKey input_key = get_keyboard_key_for(key);
	if (input_key != InputKey_Count) {
		_input.pressed[input_key] = true;
		_input.times[input_key].pressed = ENGINE_TIME;
	}
}
void key_up(int key, int modifier_flags) {
	(void) modifier_flags;
	InputKey input_key = get_keyboard_key_for(key);
	if (input_key != InputKey_Count) {
		_input.released[input_key] = true;
		_input.times[input_key].released = ENGINE_TIME;
	}
}

void mouse_down(int key) {
	InputKey input_key = get_mouse_key_for(key);
	if (input_key != InputKey_Count) {
		_input.pressed[input_key] = true;
		_input.times[input_key].pressed = ENGINE_TIME;
	}
}
void mouse_up(int key) {
	InputKey input_key = get_mouse_key_for(key);
	if (input_key != InputKey_Count) {
		_input.released[input_key] = true;
		_input.times[input_key].released = ENGINE_TIME;
	}
}

void mouse_motion(int x, int y, int xrel, int yrel) {
	_input.mouse_x = x;
	_input.mouse_y = y;
	_input.mouse_xrel = xrel;
	_input.mouse_yrel = yrel;
}
