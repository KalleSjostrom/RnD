#include "includes.h"
#include "levels.cpp"
#include "engine/utils/math/random.h"

/// SETTINGS
typedef u64 Genome;
typedef i32 Fitness;
typedef u64 PopulationIndex;

enum SelectionType {
	SelectionType_RouletteWheelSA,
	SelectionType_Truncation,
	SelectionType_Custom,
};

enum MatingType {
	MatingType_OnePointCrossover,
	MatingType_Custom,
};

enum MutationType {
	MutationType_FlipBit,
	MutationType_Custom,
};

struct GASettings {
	Fitness max_fitness;
	Fitness min_fitness;
	u64 max_iterations;
	u64 population_size;
	float mutation_probability;

	SelectionType selection_type;
	MatingType mating_type;
	MutationType mutation_type;

	void (*custom_selection)(const GASettings *settings, Fitness *population_fitness, Fitness max_fitness, Fitness min_fitness);
	void (*custom_mating)(const GASettings *settings, Genome *population, Genome *new_generation, PopulationIndex *selected, PopulationIndex selected_size);
	void (*custom_mutation)(const GASettings *settings, Genome *new_generation);
};

static const GASettings ga_settings = {
	128, 0, 4096, 1024*64, 0.1f,
	(SelectionType)0, (MatingType)0, (MutationType)0,
	0, 0, 0,
};

Genome make_random(Random &random) {
	return random_u64(random);
}

bool is_done(Fitness max, Fitness min) {
	return max == 64;
}
Fitness calculate_fitness(Genome genome) {
	return (Fitness) _mm_popcnt_u64(genome);
}
/// SETTINGS

#include "generic_genetic_algorithm.cpp"
#include "neural_net.cpp"

struct Game {
	MemoryArena persistent_arena;
	MemoryArena transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;

	EngineApi *engine;

	b32 initialized;

	i32 entity_count;
	Entity entities[512];
};

EXPORT PLUGIN_RELOAD(reload) {
	Game &game = *(Game*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
	#endif

	MemoryArena empty = {};
	game.transient_arena = empty;
	reset_arena(game.transient_arena, MB);
	globals::transient_arena = &game.transient_arena;

	reload_programs(game.components);
	setup_render_pipe(game.engine, game.render_pipe, game.components, screen_width, screen_height);
	// game.components.input.set_input(&input);

	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];

		Entity *entity = 0;
		if (i < game.entity_count) {
			entity = game.entities + i;
		} else {
			entity = game.entities + game.entity_count++;
		}

		model__set_position(game.components, *entity, V3(data.x, data.y, 0));
		model__set_rotation(game.components, *entity, data.rotation);
		model__set_scale(game.components, *entity, V3(data.w, data.h, 0));
	}
	game.entity_count = level.count;
}

EXPORT PLUGIN_UPDATE(update) {
	Game &game = *(Game*) memory;

	if (!game.initialized) {
		game.initialized = true;

		#ifdef OS_WINDOWS
			setup_gl();
		#endif
		MemoryArena empty = {};
		game.persistent_arena = empty;
		game.transient_arena = empty;
		setup_arena(game.transient_arena, MB);
		globals::transient_arena = &game.transient_arena;

		game.engine = &engine;

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		setup_programs(game.components);
		setup_render_pipe(game.engine, game.render_pipe, game.components, screen_width, screen_height);
		// game.components.input.set_input(&input);

		for (i32 i = 0; i < level.count; ++i) {
			EntityData &data = level.entity_data[i];
			Entity &entity = game.entities[game.entity_count++];

			spawn_entity(game.components, entity, data.type, V3(data.x, data.y, 0));

			model__set_position(game.components, entity, V3(data.x, data.y, 0));
			model__set_rotation(game.components, entity, data.rotation);
			model__set_scale(game.components, entity, V3(data.w, data.h, 0));
		}

		// 	// Audio
		// 	game.audio_manager.play(game.engine, "../../game/assets/test.wav");
		setup_camera(game.camera, V3(0, 0, 500), ASPECT_RATIO);
	}

	{ // Update the game
		if (IS_PRESSED(input, InputKey_Space)) {
			run(game.persistent_arena, ga_settings);
		}
		// Update all the components
		update_components(game.components, dt);
		// Handle component/component communication.
		component_glue::update(game.components, game.entities, game.entity_count, dt);
		// Update sound
		game.audio_manager.update(*globals::transient_arena, game.engine, dt);
	}

	{ // Render
		//// Render pipe ////
		render(game.render_pipe, game.components, game.camera);
	}

	// globals::transient_arena.offset = 0;
	return 0;
}
