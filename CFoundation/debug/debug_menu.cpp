namespace debug_menu{
	static const int LAYER = 200;

	static Vector4 RED = vector4( 255, 228, 63, 41 );
	static Vector4 BLUE = vector4( 255, 150, 200, 255 );
	static Vector4 WHITE = vector4( 255, 255, 255, 255 );
	static Vector4 GREEN = vector4( 255, 89, 182, 91 );
	static Vector4 YELLOW = vector4( 255, 255, 255, 0 );
	static Vector4 BLACK = vector4( 255, 0, 0, 0 );

	static const Id64 FONT = ID64(core__performance_hud__debug, "core/performance_hud/debug");
	static const float FONT_SIZE_MENU = 20.0f;
	static const Id64 MATERIAL = ID64(core__performance_hud__debug, "core/performance_hud/debug");

	static const int CATEGORY_HEIGHT = 24;
	static const int SPACING = 8;
	static const int COLUMN_COUNT = 6;
	static const int ROW_COUNT = 8;

	enum DebugMenuInput {
		DebugMenuInput_Up,
		DebugMenuInput_Down,
		DebugMenuInput_Right,
		DebugMenuInput_Left,
		DebugMenuInput_Select,
		DebugMenuInput_Toggle,
		DebugMenuInput_NextCategory,
		DebugMenuInput_PreviousCategory,
	};

	struct CommandInfo {
		char name[512];
		IdString32 event;
		bool toggleable;
		bool is_selected;
		bool is_toggled;
		void* data;
	};

	struct CategoryInfo {
		char name[512];
		CommandInfo commands[64];
		unsigned num_commands;
		unsigned selected_command_index;
		bool is_selected;
	};

	struct CommandPosition {
		unsigned col;
		unsigned row;
	};

	static unsigned get_command_index(CommandPosition pos, unsigned tile_count){ //> index [1:tile_count]
		return math::clamp((pos.row*COLUMN_COUNT) + pos.col, 0, tile_count-1);
	}

	static CommandPosition get_command_position(unsigned index){ //> col [1:COLUMN_COUNT], row [1:ROW_COUNT]
		CommandPosition pos;
		pos.col = ((index) % COLUMN_COUNT);
		pos.row = (int)floor((double)((index) / COLUMN_COUNT));
		return pos;
	}

	static void draw_tile(GuiPtr gui, char *text, Vector4 *color, float x, float y, float w, float h, MaterialPtr material){
		Vector2 position = { x, y };
		Vector2 size = { w, h };
		_Gui.rect(gui, &position, LAYER, &size, color);

		TextExtentsResult results = _Gui.text_extents(gui, text, FONT, FONT_SIZE_MENU, 0);

		int tw = (int)results.max.x - (int)results.min.x;
		int th = (int)results.max.y - (int)results.min.y;
		Vector2 text_pos = { (float)(x + (w - tw) * 0.5), (float)(y + (h - th) * 0.5) };

		_Gui.text(gui, text, FONT, FONT_SIZE_MENU, material, &text_pos, LAYER+1, 0, &BLACK);
	}

	static void draw_category_tab(GuiPtr gui, CategoryInfo *category, float x, float y, float w, float h, MaterialPtr material){
		if (category->is_selected){
			Vector2 position = { (float)x-2, (float)y-2 };
			Vector2 size = { w+4, h+4 };
			_Gui.rect(gui, &position, LAYER-1, &size, &YELLOW);
		}

		draw_tile(gui, category->name, &BLUE, x, y, w, h, material);
	}

	static void draw_command_tile(GuiPtr gui, CommandInfo *cmd, float x, float y, float w, float h, MaterialPtr material){
		Vector4 *bg_color = 0;
		if (cmd->toggleable)
			bg_color = cmd->is_toggled ? &GREEN : &RED;
		else
			bg_color = &BLUE;

		if (cmd->is_selected){
			Vector2 position = { x-2, y-2 };
			Vector2 size = { w+4, h+4 };
			_Gui.rect(gui, &position, LAYER-1, &size, &YELLOW);
		}

		draw_tile(gui, cmd->name, bg_color, x, y, w, h, material);
	}


	static void draw_commands_in_region(GuiPtr gui, CategoryInfo *category, float x, float y, float w, float h, MaterialPtr material){
		for (unsigned i = 0; i < category->num_commands; i++){
			CommandInfo *cmd = &category->commands[i];
			CommandPosition position = get_command_position(i);
			float tw = (w / COLUMN_COUNT) - SPACING;
			float th = (h / ROW_COUNT) - SPACING;
			float tx = x + (position.col) * (tw + SPACING);
			float ty = (y - (position.row) * (th + SPACING) - th);
			draw_command_tile(gui, cmd, tx, ty, tw, th, material);
		}
	}

	class DebugMenu {
	public:
		DebugMenu() : enabled(0), num_categories(0), active_category(0), selected_category(0), active_pad_index(0) {
			resolution = _Gui.resolution(0,0);
			initialize_input_context(ic);
		}

		void set_gui(GuiPtr g){
			gui = g;
			material = _Gui.material(gui, MATERIAL);
		}

		void set_event_delegate(EventDelegate *e){
			event_delegate = e;
		}

		bool is_enabled(){
			return enabled;
		}

		void set_active_category(char *name){
			for (unsigned i = 0; i < num_categories; i++){
				CategoryInfo &category = categories[i];
				if (category.name == name){
					active_category = i;
					return;
				}
			}

			active_category = num_categories++;
			CategoryInfo &category = categories[active_category];
			strcpy(category.name, name);
			category.num_commands = 0;
			category.selected_command_index = 0;
			category.is_selected = num_categories == 1 ? true : false;
		}

		void remove_all_categories(){
			num_categories = 0;
		}

		// TODO(linus): Implement this...
		// void remove_category(char *name){
		// }

		void change_command_name(IdString32 command_event, const char* name) {
			for(unsigned category = 0; category < num_categories; ++category) {
				CategoryInfo& category_info = categories[category];
				for(unsigned command = 0; command < category_info.num_commands; ++command) {
					CommandInfo& command_info = category_info.commands[command];
					if(command_info.event == command_event) {
						strcpy(command_info.name, name);
						return;
					}
				}
			}
		}

		void register_command(const char *name, IdString32 event, bool toggleable, bool is_toggled, void* data = 0) {
			CategoryInfo &category = categories[active_category];
			CommandInfo &command = category.commands[category.num_commands++];
			strcpy(command.name, name);
			command.event = event;
			command.toggleable = toggleable;
			command.is_selected = false;
			command.is_toggled = is_toggled;
			command.data = data;

			if(is_toggled) {
				event_delegate->trigger_on_debug_command_executed(command.event, command.is_toggled, command.data);
			}
		}

		void update(float dt){
			bool window_has_focus = _Window.has_focus(0) > 0;

			// Reset input
			for (int i = 0; i < ARRAY_COUNT(input); i++) {
				input[i].pressed = false;
			}

			// Toggle menu on/off
			for (int i = 0; i < ARRAY_COUNT(ic.pad_infos); i++) {
				PadInfo &info = ic.pad_infos[i];
				if (window_has_focus && pressed(info, Input_Options)){
					enabled = !enabled;
					active_pad_index = i;

					if(!enabled) {
						event_delegate->trigger_on_debug_menu_closed();
					}

					break;
				}
			}

			if (!enabled || num_categories <= 0)
				return;

			// Get inputs from active pad

			if(window_has_focus) {
				PadInfo &pad_info = ic.pad_infos[active_pad_index];
				Vector3 last_move = moves[active_pad_index];

				Vector3 move = axis(pad_info, Input_LeftStick);

				set_axis_as_pressed(pad_info, move.x >  0.5f && last_move.x <=  0.5f, input + DebugMenuInput_Right);
				set_axis_as_pressed(pad_info, move.x < -0.5f && last_move.x >= -0.5f, input + DebugMenuInput_Left);
				set_axis_as_pressed(pad_info, move.y >  0.5f && last_move.y <=  0.5f, input + DebugMenuInput_Up);
				set_axis_as_pressed(pad_info, move.y < -0.5f && last_move.y >= -0.5f, input + DebugMenuInput_Down);

				set_pressed(pad_info, input + DebugMenuInput_Right,            Input_DRight);
				set_pressed(pad_info, input + DebugMenuInput_Left,             Input_DLeft);
				set_pressed(pad_info, input + DebugMenuInput_Up,               Input_DUp);
				set_pressed(pad_info, input + DebugMenuInput_Down,             Input_DDown);
				set_pressed(pad_info, input + DebugMenuInput_Select,           Input_Cross);
				set_pressed(pad_info, input + DebugMenuInput_Toggle,           Input_Options);
				set_pressed(pad_info, input + DebugMenuInput_NextCategory,     Input_R1);
				set_pressed(pad_info, input + DebugMenuInput_PreviousCategory, Input_L1);

				moves[active_pad_index] = move;
			}

			// Get currently selected category index
			unsigned selected_category_index = 0;
			for (unsigned i = 0; i < num_categories; i++){
				CategoryInfo &category = categories[i];
				if (category.is_selected){
					selected_category_index = i;
					break;
				}
			}

			CategoryInfo *selected_category = &categories[selected_category_index];

			if (input[DebugMenuInput_NextCategory].pressed){
				selected_category_index++;
				if(selected_category_index >= num_categories) selected_category_index = 0;
			}
			else if (input[DebugMenuInput_PreviousCategory].pressed){
				if(selected_category_index == 0) {
					selected_category_index = num_categories-1;
				} else {
					selected_category_index--;
				}
			}

			// Update selected category
			selected_category->is_selected = false;
			selected_category = &categories[selected_category_index];
			selected_category->is_selected = true;

			int res_x = (int)resolution.x;
			int res_y = (int)resolution.y;
			res_x = res_x - SPACING;
			res_y = res_y;

			unsigned num_commands = selected_category->num_commands;
			if (num_commands > 0){
				unsigned selected_command_index = selected_category->selected_command_index;
				CommandInfo *selected_command_info = &selected_category->commands[selected_command_index];
				CommandPosition position = get_command_position(selected_command_index);
				if (input[DebugMenuInput_Right].pressed){
					unsigned cols_on_row = math::min(num_commands - ((position.row) * COLUMN_COUNT), COLUMN_COUNT);
					if(position.col == cols_on_row-1) {
						position.col = 0;
					} else {
						position.col += 1;
					}
				}
				else if (input[DebugMenuInput_Left].pressed){
					unsigned cols_on_row = math::min(num_commands - ((position.row) * COLUMN_COUNT), COLUMN_COUNT);
					if(position.col == 0) {
						position.col = cols_on_row-1;
					} else {
						position.col -= 1;
					}
				}

				if (input[DebugMenuInput_Up].pressed) {
					unsigned num_rows = (unsigned)ceil(double(num_commands/COLUMN_COUNT)) + 1;
					if(position.row == 0) {
						position.row = num_rows-1;
					} else {
						position.row -= 1;
					}
				} else if (input[DebugMenuInput_Down].pressed) {
					unsigned num_rows = (unsigned)ceil(double(num_commands/COLUMN_COUNT)) + 1;
					if(position.row == num_rows-1) {
						position.row = 0;
					} else {
						position.row += 1;
					}
				}

				selected_command_index = get_command_index(position, num_commands);

				selected_command_info->is_selected = false;
				selected_command_info = &selected_category->commands[selected_command_index];
				selected_category->selected_command_index = selected_command_index;
				selected_command_info->is_selected = true;

				if (input[DebugMenuInput_Select].pressed) {
					if (selected_command_info->toggleable){
						selected_command_info->is_toggled = !selected_command_info->is_toggled;
					}
					event_delegate->trigger_on_debug_command_executed(selected_command_info->event, selected_command_info->is_toggled, selected_command_info->data);
				}

				float x = SPACING;
				float y = (float)(res_y - (CATEGORY_HEIGHT + SPACING * 2));
				float w = (float)(res_x - SPACING);
				float h = y - SPACING;
				draw_commands_in_region(gui, selected_category, x, y, w, h, material);
			}

			int category_width = (res_x / num_categories);
			for (unsigned i = 0; i < num_categories; i++){
				CategoryInfo *category = &categories[i];
				float x = (float)(SPACING + i * category_width);
				float y = (float)(res_y - (CATEGORY_HEIGHT + SPACING));
				float w = (float)(category_width - SPACING);
				float h = (float)CATEGORY_HEIGHT;
				draw_category_tab(gui, category, x, y, w, h, material);
			}
		}

		// member variables
		Vector2 resolution;
		GuiPtr gui;
		MaterialPtr material;
		bool enabled;

		CategoryInfo categories[32];
		unsigned num_categories;
		unsigned active_category;
		unsigned selected_category;

		EventDelegate *event_delegate;

		// Input
		InputContext ic;
		Vector3 moves[8];
		Input input[Input_Count];
		unsigned active_pad_index;
	};
}