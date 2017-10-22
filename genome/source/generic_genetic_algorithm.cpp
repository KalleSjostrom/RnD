#include <immintrin.h>
#include "engine/utils/profiler.c"
#include "engine/utils/math/random.h"

enum ProfilerScopes {
	ProfilerScopes__run,
};

FORCE_INLINE u64 random_population_index(Random &r) {
	return random_u64(r);
}

void run(MemoryArena &arena, const GASettings &settings) {
	Random random = {};
	random_init(random, rdtsc(), 54u);

	Fitness max_fitness = settings.max_fitness;
	Fitness min_fitness = settings.min_fitness;
	u64 max_iterations = settings.max_iterations;
	PopulationIndex population_size = settings.population_size;
	float mutation_probability = settings.mutation_probability;
	u64 mutation_count = (u64)(population_size * mutation_probability);


	Genome *population = PUSH_STRUCTS(arena, population_size * 2, Genome);
	Genome *population_buffer = population + population_size;
	Fitness *fitness = PUSH_STRUCTS(arena, population_size, Fitness);
	PopulationIndex *selected = PUSH_STRUCTS(arena, population_size, PopulationIndex);

	for (u64 i = 0; i < population_size; i++) {
		population[i] = make_random(random);
	}

	PROFILER_START(run);
	for (u64 j = 0; j < max_iterations; ++j) {
		// Fitness
		Fitness min = max_fitness;
		Fitness max = min_fitness;
		for (u64 i = 0; i < population_size; i++) {
			Fitness f = calculate_fitness(population[i]);
			min = f < min ? f : min;
			max = f > max ? f : max;
			fitness[i] = f;
		}

		// End criteria
		if (is_done(max, min)) {
			LOG_INFO("GA", "YAY! Found a winner at iteration '%lu'\n", j);

			for (PopulationIndex i = 0; i < population_size; i++) {
				if (fitness[i] == max) {
					LOG_INFO("GA", "winner: %lu %lu\n", i, population[i]);
				}
			}
			break;
		}

		// Selection
		PopulationIndex selected_size = 0;
		switch (settings.selection_type) {
			// Based on the paper: "Roulette-wheel selection via stochastic acceptance" by Adam Lipowski and Dorota Lipowska.
			// http://arxiv.org/abs/1109.3627
			// Complexity: O(1).
			case SelectionType_RouletteWheelSA: {
				PopulationIndex nr_considerations = (PopulationIndex)(population_size * 0.5);
                for (u64 i = 0; i < nr_considerations; i++) {
					PopulationIndex random_index = random_population_index(random) & (population_size-1);
					float probability = (float)fitness[random_index] / (float)max;
					if (random_f32(random) <= probability) {
						selected[selected_size++] = random_index;
					}
				}
			} break;
			case SelectionType_Truncation: {
				// AboveMean
				float cutoff = min + (max-min)*0.5f;
				for (PopulationIndex i = 0; i < population_size; i++) {
					if (fitness[i] >= cutoff) {
						selected[selected_size++] = i;
					}
				}
			} break;
			case SelectionType_Custom: {
				settings.custom_selection(&settings, fitness, max_fitness, min_fitness);
			} break;
		}

		ASSERT(selected_size, "None survived the selection!");

		// Mating
		switch (settings.mating_type) {
			case MatingType_OnePointCrossover: {
				for (u64 i = 0; i < population_size; i+=2) {
					PopulationIndex mom_index = selected[random_u32(random) % selected_size];
					PopulationIndex dad_index = selected[random_u32(random) % selected_size];

					Genome mom = population[mom_index];
					Genome dad = population[dad_index];

					i32 point = random_u32(random) & (64 - 1);
					Genome mask = 0xFFFFFFFFFFFFFFFF << point;

					population_buffer[i] = (mask & mom) | (~mask & dad);
					population_buffer[i+1] = (mask & dad) | (~mask & mom);
				}
			} break;
			case MatingType_Custom: {
				settings.custom_mating(&settings, population, population_buffer, selected, selected_size);
			} break;
		}

		// Mutation
		switch (settings.mutation_type) {
			case MutationType_FlipBit: {
				for (u64 i = 0; i < mutation_count; i++) {
					u64 random_index = random_population_index(random) & (population_size-1);

					Genome individual = population_buffer[random_index];

					i32 point = random_u32(random) & (64 - 1);
					individual ^= (Genome)(1 << point);

					population_buffer[random_index] = individual;
				}
			} break;
			case MutationType_Custom: {
				settings.custom_mutation(&settings, population_buffer);
			} break;
		}

		// These are genetic operators, there can be any number of them.. How to extend??
		// it is possible to use other operators such as regrouping, colonization-extinction, or migration in genetic algorithms
		// parameters such as the mutation probability, crossover probability
		// increasing the probability of mutation when the solution quality drops (called triggered hypermutation)
		// occasionally introducing entirely new, randomly generated elements into the gene pool (called random immigrants)

		Genome *temp = population_buffer;
		population_buffer = population;
		population = temp;
	}
	PROFILER_STOP(run);
	PROFILER_PRINT(run);
}
