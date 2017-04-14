#include <immintrin.h>
#include "../utils/profiler.c"
#include "../utils/common.h"
#include "../utils/string_utils.inl"

#include "strings.cpp"

#define EMPTY ' '
#define DIM 32
#define GRID_INDEX(row, col) ((row) * DIM + (col))
#define CHAR_AT(row, col) (grid[GRID_INDEX(row, col)])

void render_horizontal(char *grid, int row, int col, String *string) {
	for (int i = 0; i < string->length; i++) {
		grid[GRID_INDEX(row, col+i)] = string->text[i];
	}
}
void render_vertical(char *grid, int row, int col, String *string) {
	for (int i = 0; i < string->length; i++) {
		grid[GRID_INDEX(row+i, col)] = string->text[i];
	}
}

bool fits_at_horizontal(char *grid, int row, int col, String *string) {
	for (int i = 0; i < string->length; i++, col++) { // Advance to the right each step
		char c_middle = CHAR_AT(row, col);
		if ((*string)[i] == c_middle)
			continue;

		char c_under = CHAR_AT(row + 1, col);
		char c_above = CHAR_AT(row - 1, col);

		char c_left  = CHAR_AT(row, col - 1);
		char c_right = CHAR_AT(row, col + 1);

		bool under = c_under == EMPTY;
		bool above = c_above == EMPTY;

		bool left = c_left == EMPTY;
		if (i > 0) {
			left = left || c_left == (*string)[i-1];
		}
		bool right = c_right == EMPTY;
		if (i < string->length-1) {
			right = right || c_right == (*string)[i+1];
		}

		bool valid = under && above && left && right;
		if (!valid)
			return false;
	}

	return true;
}

bool fits_at_vertical(char *grid, int row, int col, String *string) {
	for (int i = 0; i < string->length; i++, row++) {
		char c_middle = CHAR_AT(row, col);
		if ((*string)[i] == c_middle)
			continue;

		char c_under = CHAR_AT(row + 1, col);
		char c_above = CHAR_AT(row - 1, col);

		char c_left = CHAR_AT(row, col - 1);
		char c_right = CHAR_AT(row, col + 1);

		bool left = c_left == EMPTY;
		bool right = c_right == EMPTY;

		bool above = c_above == EMPTY;
		if (i > 0) {
			above = above || c_above == (*string)[i-1];
		}
		bool under = c_under == EMPTY;
		if (i < string->length-1) {
			under = under || c_under == (*string)[i+1];
		}

		bool valid = under && above && left && right;
		if (!valid)
			return false;
	}

	return true;
}

struct WordPosition {
	int row;
	int col;
	bool horizontal;
};
struct Choice {
	int selected;
	int options;
};

// 78900352
int test_string(char *grid, String *string, WordPosition *positions) {
	int num_positions = 0;

	for (int i = 0; i < string->length; i++) {
		char sc = (*string)[i];
		for (int row = 0; row < DIM; row++) {
			for (int col = 0; col < DIM; col++) {
				char c = CHAR_AT(row, col);
				if (c == sc) {
					if (CHAR_AT(row, col+1) == EMPTY && CHAR_AT(row, col-1) == EMPTY) {
						if (fits_at_horizontal(grid, row, col - i, string)) {
							WordPosition pos = { row, col - i, true };
							positions[num_positions++] = pos;
						}
					} else if (CHAR_AT(row+1, col) == EMPTY && CHAR_AT(row-1, col) == EMPTY) {
						if (fits_at_vertical(grid, row - i, col, string)) {
							WordPosition pos = { row - i, col, false };
							positions[num_positions++] = pos;
						}
					}
				}
			}
		}
	}

	return num_positions;
}

inline String *pick_word(int *words_left, int *words_left_size, Choice *choice) {
	int size = *words_left_size;
	int index = choice->selected == -1 ? (random() % size) : choice->selected;
	int string_index = words_left[index];
	words_left[index] = words_left[--size];
	String *word = strings + string_index;
	*words_left_size = size;

	choice->selected = index;
	choice->options = size;

	return word;
}

struct Genome {
	Choice choices[ARRAY_SIZE(strings)*2];
	u32 num_choices;
};

struct Dunno {
	Genome *genome;
	Choice choice;
	bool premade;
};

inline void add_choice(Genome *genome, Dunno *dunno) {
	if (!dunno->premade)
		genome->choices[genome->num_choices++] = dunno->choice;
}

void init_dunno(Dunno *dunno, int choice_tracker) {
	dunno->choice.selected = -1;
	dunno->premade = false;
	if (dunno->genome->num_choices > choice_tracker) {
		dunno->choice = dunno->genome->choices[choice_tracker];
		dunno->premade = true;
	}
}

int calculate_fitness(char *grid, Genome *genome, bool render = false) {
	int words_left[ARRAY_SIZE(strings)];
	int words_left_size = ARRAY_SIZE(strings);
	for (int i = 0; i < ARRAY_SIZE(strings); ++i)
		words_left[i] = i;

	Dunno dunno;
	dunno.genome = genome;
	int choice_tracker = 0;

	init_dunno(&dunno, choice_tracker++);
	String *word = pick_word(words_left, &words_left_size, &dunno.choice);
	add_choice(genome, &dunno);

	render_horizontal(grid, DIM/2, (DIM - word->length)/2, word);

	for (int i = 0; i < ARRAY_SIZE(strings)/2; i++) {
		init_dunno(&dunno, choice_tracker++);
		word = pick_word(words_left, &words_left_size, &dunno.choice);
		add_choice(genome, &dunno);

		WordPosition positions[1024];
		int num_positions = test_string(grid, word, positions);

		init_dunno(&dunno, choice_tracker++);

		Choice &choice = dunno.choice;
		if (num_positions > 0) {
			int position_index = choice.selected == -1 ? (random() % num_positions) : choice.selected;

			choice.selected = position_index;
			choice.options = num_positions;

			add_choice(genome, &dunno);

			WordPosition &position = positions[position_index];
			if (position.horizontal) {
				render_horizontal(grid, position.row, position.col, word);
			} else {
				render_vertical(grid, position.row, position.col, word);
			}
		} else {
			if (!dunno.premade) {
				choice.selected = 0;
				choice.options = 0;
				add_choice(genome, &dunno);
			}
		}
	}

#if 0
	int mincol = DIM;
	int maxcol = -DIM;
	int minrow = DIM;
	int maxrow = -DIM;
	for (int row = 0; row < DIM; row++) {
		for (int col = 0; col < DIM; col++) {
			char c = CHAR_AT(row, col);
			if (c != EMPTY) {
				mincol = col < mincol ? col : mincol;
				minrow = row < minrow ? row : minrow;
				maxcol = col > maxcol ? col : maxcol;
				maxrow = row > maxrow ? row : maxrow;
			}
		}
	}
#else
	int mincol = 0;
	int maxcol = DIM-1;
	int minrow = 0;
	int maxrow = DIM-1;
#endif

	u32 num_empty = 0;
	for (int row = minrow; row <= maxrow; row++) {
		for (int col = mincol; col <= maxcol; col++) {
			char c = CHAR_AT(row, col);
			num_empty += (c == EMPTY);
		}
	}

	if (render) {
		for (int row = minrow; row <= maxrow; row++) {
			printf("%2d ", row);
			for (int col = mincol; col <= maxcol; col++) {
				putc(grid[GRID_INDEX(row, col)], stdout);
				putc(',', stdout);
			}
			putc('\n', stdout);
		}
	}

	return num_empty;
}

#define MAX_ITERATIONS 1
#define POPULATION_SIZE 200
#define MUTATION_PROBABILITY 0.1f
static u32 NR_MUTATIONS = (u32)(POPULATION_SIZE * MUTATION_PROBABILITY);

int main(int argc, char const *argv[]) {
	int seed = rdtsc();
	printf("%d\n", seed);
	srandom(seed);

	Genome *population        = (Genome*)malloc(POPULATION_SIZE * sizeof(Genome));
	Genome *population_buffer = (Genome*)malloc(POPULATION_SIZE * sizeof(Genome));

	int *fitness  = (int*)malloc(POPULATION_SIZE * sizeof(int));
	u32 *selected = (u32*)malloc(POPULATION_SIZE * sizeof(u32));

	for (int i = 0; i < POPULATION_SIZE; i++) {
		population[i].num_choices = 0;
		population_buffer[i].num_choices = 0;
	}

	char *grid = (char*)malloc(DIM*DIM*sizeof(char));

	for (int j = 0; j < MAX_ITERATIONS; ++j)
	{
		printf("Iteration: %d\n", j);

		int min = 1000000;
		int max = 0;
		for (int i = 0; i < POPULATION_SIZE; i++) {

			// Prepare grid
			for (int i = 0; i < DIM*DIM; i++) {
				grid[i] = EMPTY;
			}

			// Try it out!
			int f = calculate_fitness(grid, population + i);
			min = f < min ? f : min;
			max = f > max ? f : max;
			fitness[i] = f;
		}

		// Selection
		u32 selected_size = 0;
		float cutoff = min + (max-min)*0.5f; // lerp(min, max, 0.5f)
		for (int i = 0; i < POPULATION_SIZE; i++) {
			if (fitness[i] <= cutoff) {
				selected[selected_size++] = i;
			}
		}

		// Reproduction
		for (int i = 0; i < POPULATION_SIZE; i++) {
			u32 index = selected[random() % selected_size];

			Genome *individual = population + index;

			// Reproduce by cloning...
			population_buffer[i] = *individual;

			// ...with mutation
			Genome &child = population_buffer[i];
			int point = random() % child.num_choices;
			child.num_choices = point;
		}

		Genome *temp = population_buffer;
		population_buffer = population;
		population = temp;
	}

	for (int i = 0; i < POPULATION_SIZE; i++) {
		// Prepare grid
		for (int i = 0; i < DIM*DIM; i++) {
			grid[i] = EMPTY;
		}

		// Print it out
		if (fitness[i] <= 10000) {
			printf("------------ %d ------------\n", fitness[i]);
			int f = calculate_fitness(grid, population_buffer + i, true);
		}
	}

	return 0;
}
