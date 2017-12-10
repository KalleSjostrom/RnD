namespace game {
	struct EventDelegate;
}

#if defined(PS4)
	#undef _Window
	#undef _PS4Pad
	#undef _Mouse
	#undef _Keyboard
#else
	#undef _Psn
#endif

struct GlobalTable {
	bool PAUSED;
	float TIME_SCALE;

	float ENGINE_DT;
	float GAME_DT;

	float ENGINE_TIME;
	float GAME_TIME;
};

namespace globals {
	GlobalTable *global_table;
	Random randomizer;
	int64_t active_threads;
}
#define _G (*globals::global_table)

namespace game {
	#include "game_strings/game_strings.h"
	#include "generated/asset_strings.generated.cpp"
	#include "generated/build_info.generated.h"
	#include "game/game.cpp"

	struct ReloaderEntryPoint {
		GlobalTable *global_table;
		Game *game;
	};
}