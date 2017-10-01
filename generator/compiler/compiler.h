enum Location {
	Location_Content    = 1 << 0,
	Location_Foundation = 1 << 1,
	Location_Code       = 1 << 2,
	Location_Packages   = 1 << 3,
	Location_Rendering  = 1 << 4,

	Location_Hooks      = 1 << 5,
	Location_Root       = 1 << 6,
	Location_Scripts    = 1 << 7,

	Location_Generated  = 1 << 8,
};

static const String _abilities            = MAKE_STRING("abilities");
static const String _generated            = MAKE_STRING("generated");
static const String _hg                   = MAKE_STRING(".hg");
static const String _boot                 = MAKE_STRING("boot");
static const String _codegen              = MAKE_STRING("codegen");
static const String _debug                = MAKE_STRING("debug");
static const String _flow                 = MAKE_STRING("flow");
static const String _game_strings         = MAKE_STRING("game_strings");
static const String _gui                  = MAKE_STRING("gui");
static const String _plugin_environment   = MAKE_STRING("plugin_environment");
static const String _precompiled          = MAKE_STRING("precompiled");
static const String _reload               = MAKE_STRING("reload");
static const String _scripts              = MAKE_STRING("scripts");
static const String _tools                = MAKE_STRING("tools");
static const String _utils                = MAKE_STRING("utils");
static const String _art_shared           = MAKE_STRING("art_shared");
static const String _animations           = MAKE_STRING("animations");
static const String _animation            = MAKE_STRING("animation");
static const String _textures             = MAKE_STRING("textures");
static const String _materials            = MAKE_STRING("materials");
static const String _network              = MAKE_STRING("network");
static const String _shading_environments = MAKE_STRING("shading_environments");
static const String _code                 = MAKE_STRING("code");
static const String _content              = MAKE_STRING("content");

bool ignored_directory(String &directory) {
	return
		// Code folder
		is_equal(directory, _generated) ||

		// Foundation folder
		is_equal(directory, _hg) ||
		is_equal(directory, _codegen) ||
		is_equal(directory, _plugin_environment) ||
		is_equal(directory, _scripts) ||

		// Content folder
		is_equal(directory, _art_shared) ||
		is_equal(directory, _animation) // Animation controllers are the only thing we care about (atm), and they are never put inside an animation folder
		;
}

enum FileType {
	FileType_None               = 1 <<  0,

	FileType_H                  = 1 <<  1,
	FileType_Cpp                = 1 <<  2,

	FileType_Conversions        = 1 <<  3,
	FileType_GameStateMachine   = 1 <<  4,

	FileType_Entity             = 1 <<  5,
	FileType_Unit               = 1 <<  6,
	FileType_ShadingEnvironment = 1 <<  7,
	FileType_Font               = 1 <<  8,
	FileType_Package            = 1 <<  9,
	FileType_Material           = 1 << 10,
	FileType_Particles          = 1 << 11,

	FileType_Flow               = 1 << 12,
	FileType_Level              = 1 << 13,
	FileType_AnimController     = 1 << 14,

	FileType_Py                 = 1 << 15,
	FileType_Nfo                = 1 << 16,
	FileType_BuildInfo          = 1 << 17,

	FileType_AbilityNodes       = 1 << 18,
	FileType_BehaviorNodes      = 1 << 19,

	FileType_Abilities          = 1 << 20,
	FileType_Behavior           = 1 << 21,
};

static const String _ending_h                   = MAKE_STRING(".h");
static const String _ending_cpp                 = MAKE_STRING(".cpp");

static const String _ending_conversions         = MAKE_STRING(".conversions");
static const String _ending_game_state_machine  = MAKE_STRING(".game_state_machine");

static const String _ending_entity              = MAKE_STRING(".entity");
static const String _ending_unit                = MAKE_STRING(".unit");
static const String _ending_shading_environment = MAKE_STRING(".shading_environment");
static const String _ending_font                = MAKE_STRING(".font");
static const String _ending_package             = MAKE_STRING(".package");
static const String _ending_material            = MAKE_STRING(".material");
static const String _ending_particles           = MAKE_STRING(".particles");

static const String _ending_flow                = MAKE_STRING(".flow");
static const String _ending_level               = MAKE_STRING(".level");
static const String _ending_anim_controller     = MAKE_STRING(".anim_controller");

static const String _ending_component           = MAKE_STRING(".component");
static const String _ending_component_h         = MAKE_STRING("_component.h");

static const String _ending_py                  = MAKE_STRING(".py");
static const String _ending_nfo                 = MAKE_STRING(".nfo");
static const String _ending_build_info          = MAKE_STRING(".build_info");

static const String _ending_ability_nodes       = MAKE_STRING(".ability_nodes");
static const String _ending_behavior_nodes      = MAKE_STRING(".behavior_nodes");

static const String _ending_abilities           = MAKE_STRING(".abilities");
static const String _ending_behavior            = MAKE_STRING(".behavior");

FileType get_file_type(String &filename) {
	if (ends_in(filename, _ending_h))                   { return FileType_H;                  }
	if (ends_in(filename, _ending_cpp))                 { return FileType_Cpp;                }

	if (ends_in(filename, _ending_conversions))         { return FileType_Conversions;        }
	if (ends_in(filename, _ending_game_state_machine))  { return FileType_GameStateMachine;   }

	if (ends_in(filename, _ending_entity))              { return FileType_Entity;             }
	if (ends_in(filename, _ending_unit))                { return FileType_Unit;               }
	if (ends_in(filename, _ending_shading_environment)) { return FileType_ShadingEnvironment; }
	if (ends_in(filename, _ending_font))                { return FileType_Font;               }
	if (ends_in(filename, _ending_package))             { return FileType_Package;            }
	if (ends_in(filename, _ending_material))            { return FileType_Material;           }
	if (ends_in(filename, _ending_particles))           { return FileType_Particles;          }

	if (ends_in(filename, _ending_flow))                { return FileType_Flow;               }
	if (ends_in(filename, _ending_level))               { return FileType_Level;              }
	if (ends_in(filename, _ending_anim_controller))     { return FileType_AnimController;     }

	if (ends_in(filename, _ending_py))                  { return FileType_Py;                 }
	if (ends_in(filename, _ending_nfo))                 { return FileType_Nfo;                }
	if (ends_in(filename, _ending_build_info))          { return FileType_BuildInfo;          }

	if (ends_in(filename, _ending_ability_nodes))       { return FileType_AbilityNodes;       }
	if (ends_in(filename, _ending_behavior_nodes))      { return FileType_BehaviorNodes;      }

	if (ends_in(filename, _ending_abilities))           { return FileType_Abilities;          }
	if (ends_in(filename, _ending_behavior))            { return FileType_Behavior;           }

	return FileType_None;
}
