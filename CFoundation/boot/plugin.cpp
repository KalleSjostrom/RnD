#include "boot.cpp"

extern "C" {
	#define GET_PLUGIN_API PLUGIN_DLLEXPORT void *get_plugin_api

	GET_PLUGIN_API(unsigned api) {
		if (api == PLUGIN_API_ID) {
			static struct PluginApi api = {0};

			api.setup_game    = &game::setup_game;
			api.start_reload  = &game::start_reload;
			api.finish_reload = &game::finish_reload;
			api.update_game   = &game::update;
			api.render_game   = &game::render;
			api.shutdown_game = &game::shutdown;

			return &api;
		}
		return 0;
	}
}