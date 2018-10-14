#include "common.h"

#include "include/SDL.h"

#include "input_api.h"
#include "input.h"

InputKey get_keyboard_key_for(i32 key) {
	switch (key) {
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
		default: { return InputKey_Count; }; break;
	};
}

InputKey get_mouse_key_for(i32 key) {
	switch (key) {
		case SDL_BUTTON_LEFT: { return InputKey_MouseLeft; } break;
		case SDL_BUTTON_MIDDLE: { return InputKey_MouseMiddle; } break;
		case SDL_BUTTON_RIGHT: { return InputKey_MouseRight; } break;
		case SDL_BUTTON_X1: { return InputKey_MouseX1; } break;
		case SDL_BUTTON_X2: { return InputKey_MouseX2; } break;

		default: { return InputKey_Count; }; break;
	}
}

void key_down(InputData &input, u64 time, i32 key, i32 modifier_flags) {
	(void) modifier_flags;
	InputKey input_key = get_keyboard_key_for(key);
	if (input_key != InputKey_Count) {
		input.pressed[input_key] = true;
		input.times[input_key].pressed = time;
	}
}
void key_up(InputData &input, u64 time, i32 key, i32 modifier_flags) {
	(void) modifier_flags;
	InputKey input_key = get_keyboard_key_for(key);
	if (input_key != InputKey_Count) {
		input.released[input_key] = true;
		input.times[input_key].released = time;
	}
}

void mouse_down(InputData &input, u64 time, i32 key) {
	InputKey input_key = get_mouse_key_for(key);
	if (input_key != InputKey_Count) {
		input.pressed[input_key] = true;
		input.times[input_key].pressed = time;
	}
}
void mouse_up(InputData &input, u64 time, i32 key) {
	InputKey input_key = get_mouse_key_for(key);
	if (input_key != InputKey_Count) {
		input.released[input_key] = true;
		input.times[input_key].released = time;
	}
}

void mouse_motion(InputData &input, i32 x, i32 y, i32 xrel, i32 yrel) {
	input.mouse_x = x;
	input.mouse_y = y;
	input.mouse_xrel = xrel;
	input.mouse_yrel = yrel;
}
