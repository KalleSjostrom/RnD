#pragma once

InputKey get_keyboard_key_for(i32 key);
InputKey get_mouse_key_for(i32 key);

void key_down(InputData &input, u64 time, i32 key, i32 modifier_flags);
void key_up(InputData &input, u64 time, i32 key, i32 modifier_flags);
void mouse_down(InputData &input, u64 time, i32 key);
void mouse_up(InputData &input, u64 time, i32 key);
void mouse_motion(InputData &input, i32 x, i32 y, i32 xrel, i32 yrel);
