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

struct Application {
	ArenaAllocator persistent_arena;
	ArenaAllocator transient_arena;

	ComponentGroup components;
	AudioManager audio_manager;
	RenderPipe render_pipe;
	Camera camera;

	EngineApi *engine;

	bool initialized;

	i32 entity_count;
	Entity entities[512];
};

EXPORT PLUGIN_RELOAD(reload) {
	Application &application = *(Application*) memory;

	#ifdef OS_WINDOWS
		setup_gl();
	#endif

	ArenaAllocator empty = {};
	application.transient_arena = empty;
	reset_arena(application.transient_arena, MB);
	globals::transient_arena = &application.transient_arena;

	reload_programs(application.components);
	setup_render_pipe(application.engine, application.render_pipe, application.components, screen_width, screen_height);
	application.components.input.set_input_data(&input);

	Level level = make_level();
	for (i32 i = 0; i < level.count; ++i) {
		EntityData &data = level.entity_data[i];

		Entity *entity = 0;
		if (i < application.entity_count) {
			entity = application.entities + i;
		} else {
			entity = application.entities + application.entity_count++;
		}

		model__set_position(application.components, *entity, data.offset);
		model__set_rotation(application.components, *entity, data.rotation);
		model__set_scale(application.components, *entity, data.size);
	}
	application.entity_count = level.count;
}

EXPORT PLUGIN_UPDATE(update) {
	Application &application = *(Application*) memory;

	if (!application.initialized) {
		application.initialized = true;

		#ifdef OS_WINDOWS
			setup_gl();
		#endif
		ArenaAllocator empty = {};
		application.persistent_arena = empty;
		application.transient_arena = empty;
		setup_arena(application.transient_arena, MB);
		globals::transient_arena = &application.transient_arena;

		application.engine = &engine;

		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		// glEnable(GL_DEPTH_TEST);

		setup_programs(application.components);
		setup_render_pipe(application.engine, application.render_pipe, application.components, screen_width, screen_height);
		application.components.input.set_input_data(&input);

		Level level = make_level();
		for (i32 i = 0; i < level.count; ++i) {
			EntityData &data = level.entity_data[i];
			Entity &entity = application.entities[application.entity_count++];

			spawn_entity(application.components, entity, data.type, data.context, data.offset);

			model__set_position(application.components, entity, data.offset);
			model__set_rotation(application.components, entity, data.rotation);
			model__set_scale(application.components, entity, data.size);
		}

		// 	// Audio
		// 	application.audio_manager.play(application.engine, "../../application/assets/test.wav");
		setup_camera(application.camera, V3(0, 0, 500), ASPECT_RATIO);
	}

	{ // Update the application
		if (is_pressed(input, InputKey_Space)) {
			run(application.persistent_arena, ga_settings);
		}
		// Update all the components
		update_components(application.components, dt);
		// Handle component/component communication.
		component_glue::update(application.components, application.entities, application.entity_count, dt);
		// Update sound
		application.audio_manager.update(*globals::transient_arena, application.engine, dt);
	}

	{ // Render
		//// Render pipe ////
		render(application.render_pipe, application.components, application.camera);
	}

	// globals::transient_arena.offset = 0;
	return 0;
}
