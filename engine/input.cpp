InputKey get_input_key_for(int key) {
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

static InputData _input = {};

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



#if 0
Zero	0	29
One	1	18
Two	2	19
Three	3	20
Four	4	21
Five	5	23
Six	6	22
Seven	7	26
Eight	8	28
Nine	9	25
A	0
B	11
C	8
D	2
E	14
F	3
G	5
H	4
I	34
J	38
K	40
L	37
M	46
N	45
O	31
P	35
Q	12
R	15
S	1
T	17
U	32
V	9
W	13
X	7
Y	16
Z	6
SectionSign	§	10
Grave	`	50
Minus	-	27
Equal	=	24
LeftBracket	[	33
RightBracket	]	30
Semicolon	;	41
Quote	'	39
Comma	,	43
Period	.	47
Slash	/	44
Backslash	\	42
Keypad0	0	82
Keypad1	1	83
Keypad2	2	84
Keypad3	3	85
Keypad4	4	86
Keypad5	5	87
Keypad6	6	88
Keypad7	7	89
Keypad8	8	91
Keypad9	9	92
KeypadDecimal	.	65
KeypadMultiply	*	67
KeypadPlus	+	69
KeypadDivide	/	75
KeypadMinus	-	78
KeypadEquals	=	81
KeypadClear	⌧	71
KeypadEnter	⌤	76
Space	␣	49
Return	⏎	36
Tab	⇥	48
Delete	⌫	51
ForwardDelete	⌦	117
Linefeed ?	␊	52
Escape	⎋	53
Command	⌘	55
Shift	⇧	56
CapsLock	⇪	57
Option	⌥	58
Control	⌃	59
RightShift	⇧	60
RightOption	⌥	61
RightControl	⌃	62
Function	fn	63
F1	122
F2	120
F3	99
F4	118
F5	96
F6	97
F7	98
F8	100
F9	101
F10	109
F11	103
F12	111
F13	105
BrightnessDown	F14	107
BrightnessUp	F15	113
F16	106
F17	64
F18	79
F19	80
F20	90
VolumeUp ?		72
VolumeDown ?		73
Mute ?		74
Help/Insert	?	114
Home	⇱	115
End	⇲	119
PageUp	⇞	116
PageDown	⇟	121
LeftArrow	←	123
RightArrow	→	124
DownArrow	↓	125
UpArrow	↑	126
#endif
