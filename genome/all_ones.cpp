#include <immintrin.h>
#include "../utils/profiler.c"
#include "../utils/common.h"

enum ProfilerScopes {
	ProfilerScopes__run,
};



// _rdrand32_step()

/// SETTINGS
#define MAX_ITERATIONS 4096
#define POPULATION_SIZE 1024*64
#define MUTATION_PROBABILITY 0.1f
static u32 NR_MUTATIONS = (u32)(POPULATION_SIZE * MUTATION_PROBABILITY);
int calculate_fitness(u32 genome) {
	return _mm_popcnt_u32(genome);
}
/// SETTINGS


int main(int argc, char const *argv[]) {
	int seed = 0; // rdtsc();
	srandom(seed);

	u32 *population = (u32*)malloc(POPULATION_SIZE * sizeof(u32));
	u32 *population_buffer = (u32*)malloc(POPULATION_SIZE * sizeof(u32));
	int *fitness = (int*)malloc(POPULATION_SIZE * sizeof(int));
	u32 *selected = (u32*)malloc(POPULATION_SIZE * sizeof(u32));

	for (int i = 0; i < POPULATION_SIZE; i++) {
		population[i] = random();
	}

	PROFILER_START(run);
	for (int j = 0; j < MAX_ITERATIONS; ++j)
	{
		// Fitness
		int min = 1000;
		int max = 0;
		for (int i = 0; i < POPULATION_SIZE; i++) {
			int f = calculate_fitness(population[i]);
			min = f < min ? f : min;
			max = f > max ? f : max;
			fitness[i] = f;
		}

		if (max == 32) {
			printf("YAY! Found a 32! %d\n", j);
			break;
		}

		// Selection
		u32 selected_size = 0;
		float cutoff = min + (max-min)*0.5f; // lerp(min, max, 0.5f)
		for (int i = 0; i < POPULATION_SIZE; i++) {
			if (fitness[i] >= cutoff) {
				selected[selected_size++] = i;
			}
		}

		//printf("%d %d\n", min, max);
		//printf("%d %.3f\n", selected_size, cutoff);

		// Mating
		// One point crossover
		for (int i = 0; i < POPULATION_SIZE; i+=2) {
			u32 mom_index = selected[random() % selected_size];
			u32 dad_index = selected[random() % selected_size];

			u32 mom = population[mom_index];
			u32 dad = population[dad_index];

			u32 mask;

			mask = 0xFFFFFFFF << (random() % 32);
			population_buffer[i] = (mask & mom) | (~mask & dad);

			mask = 0xFFFFFFFF << (random() % 32);
			population_buffer[i+1] = (mask & mom) | (~mask & dad);
		}

		// Mutation
		for (int i = 0; i < NR_MUTATIONS; i++) {
			u32 random_index = random() % POPULATION_SIZE;

			u32 individual = population_buffer[random_index];

			int point = random() % 32;
			individual ^= (1 << point);

			population_buffer[random_index] = individual;
		}


		// These are genetic operators, there can be any number of them.. How to extend??
		// it is possible to use other operators such as regrouping, colonization-extinction, or migration in genetic algorithms
		// parameters such as the mutation probability, crossover probability

		// increasing the probability of mutation when the solution quality drops (called triggered hypermutation)
		// occasionally introducing entirely new, randomly generated elements into the gene pool (called random immigrants)

		u32 *temp = population_buffer;
		population_buffer = population;
		population = temp;
	}
	PROFILER_STOP(run);
	PROFILER_PRINT(run);

	return 0;
}
