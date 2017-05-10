#include <immintrin.h>
#include "../utils/common.h"
#include "../utils/profiler.c"
#include "engine/utils/string.h"

#include "strings.cpp"

#define EMPTY ' '
#define DIM 32
#define GRID_INDEX(row, col) ((row) * DIM + (col))
#define CHAR_AT(row, col) (grid[GRID_INDEX(row, col)])

void render_horizontal(char *grid, i32 row, i32 col, String *string) {
	for (i32 i = 0; i < string->length; i++) {
		grid[GRID_INDEX(row, col+i)] = string->text[i];
	}
}
void render_vertical(char *grid, i32 row, i32 col, String *string) {
	for (i32 i = 0; i < string->length; i++) {
		grid[GRID_INDEX(row+i, col)] = string->text[i];
	}
}

bool fits_at_horizontal(char *grid, i32 row, i32 col, String *string) {
	for (i32 i = 0; i < string->length; i++, col++) { // Advance to the right each step
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

bool fits_at_vertical(char *grid, i32 row, i32 col, String *string) {
	for (i32 i = 0; i < string->length; i++, row++) {
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
	i32 row;
	i32 col;
	b32 horizontal;
};
struct Choice {
	i32 selected;
	i32 options;
};

// 78900352
i32 test_string(char *grid, String *string, WordPosition *positions) {
	i32 num_positions = 0;

	for (i32 i = 0; i < string->length; i++) {
		char sc = (*string)[i];
		for (i32 row = 0; row < DIM; row++) {
			for (i32 col = 0; col < DIM; col++) {
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

inline String *pick_word(i32 *words_left, i32 *words_left_size, Choice *choice) {
	i32 size = *words_left_size;
	i32 index = choice->selected == -1 ? (random() % size) : choice->selected;
	i32 string_index = words_left[index];
	words_left[index] = words_left[--size];
	String *word = strings + string_index;
	*words_left_size = size;

	choice->selected = index;
	choice->options = size;

	return word;
}

struct Genome {
	Choice choices[ARRAY_COUNT(strings)*2];
	i32 num_choices;
};

struct Dunno {
	Genome *genome;
	Choice choice;
	b32 premade;
	i32 __padding;
};

inline void add_choice(Genome *genome, Dunno *dunno) {
	if (!dunno->premade)
		genome->choices[genome->num_choices++] = dunno->choice;
}

void init_dunno(Dunno *dunno, i32 choice_tracker) {
	dunno->choice.selected = -1;
	dunno->premade = false;
	if (dunno->genome->num_choices > choice_tracker) {
		dunno->choice = dunno->genome->choices[choice_tracker];
		dunno->premade = true;
	}
}

i32 calculate_fitness(char *grid, Genome *genome, bool render = false) {
	static const i32 string_length = (i32) ARRAY_COUNT(strings);
	i32 words_left[string_length];
	i32 words_left_size = string_length;
	for (i32 i = 0; i < string_length; ++i)
		words_left[i] = i;

	Dunno dunno;
	dunno.genome = genome;
	i32 choice_tracker = 0;

	init_dunno(&dunno, choice_tracker++);
	String *word = pick_word(words_left, &words_left_size, &dunno.choice);
	add_choice(genome, &dunno);

	render_horizontal(grid, DIM/2, (DIM - word->length)/2, word);

	for (i32 i = 0; i < string_length/2; i++) {
		init_dunno(&dunno, choice_tracker++);
		word = pick_word(words_left, &words_left_size, &dunno.choice);
		add_choice(genome, &dunno);

		WordPosition positions[1024];
		i32 num_positions = test_string(grid, word, positions);

		init_dunno(&dunno, choice_tracker++);

		Choice &choice = dunno.choice;
		if (num_positions > 0) {
			i32 position_index = choice.selected == -1 ? (random() % num_positions) : choice.selected;

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
	i32 mincol = DIM;
	i32 maxcol = -DIM;
	i32 minrow = DIM;
	i32 maxrow = -DIM;
	for (i32 row = 0; row < DIM; row++) {
		for (i32 col = 0; col < DIM; col++) {
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
	i32 mincol = 0;
	i32 maxcol = DIM-1;
	i32 minrow = 0;
	i32 maxrow = DIM-1;
#endif

	i32 num_empty = 0;
	for (i32 row = minrow; row <= maxrow; row++) {
		for (i32 col = mincol; col <= maxcol; col++) {
			char c = CHAR_AT(row, col);
			num_empty += (c == EMPTY);
		}
	}

	if (render) {
		for (i32 row = minrow; row <= maxrow; row++) {
			printf("%2d ", row);
			for (i32 col = mincol; col <= maxcol; col++) {
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

i32 main(i32 argc, char const *argv[]) {
	(void)argc;
	(void)argv;
	u64 seed = rdtsc();
	printf("%llu\n", seed);
	srandom((u32) seed);

	Genome *population        = (Genome*)malloc(POPULATION_SIZE * sizeof(Genome));
	Genome *population_buffer = (Genome*)malloc(POPULATION_SIZE * sizeof(Genome));

	i32 *fitness  = (i32*)malloc(POPULATION_SIZE * sizeof(i32));
	i32 *selected = (i32*)malloc(POPULATION_SIZE * sizeof(i32));

	for (i32 i = 0; i < POPULATION_SIZE; i++) {
		population[i].num_choices = 0;
		population_buffer[i].num_choices = 0;
	}

	char *grid = (char*)malloc(DIM*DIM*sizeof(char));

	for (i32 j = 0; j < MAX_ITERATIONS; ++j)
	{
		printf("Iteration: %d\n", j);

		i32 min = 1000000;
		i32 max = 0;
		for (i32 i = 0; i < POPULATION_SIZE; i++) {
			// Prepare grid
			memset(grid, EMPTY, DIM*DIM);

			// Try it out!
			i32 f = calculate_fitness(grid, population + i);
			min = f < min ? f : min;
			max = f > max ? f : max;
			fitness[i] = f;
		}

		// Selection
		i32 selected_size = 0;
		float cutoff = min + (max-min)*0.5f; // lerp(min, max, 0.5f)
		for (i32 i = 0; i < POPULATION_SIZE; i++) {
			if (fitness[i] <= cutoff) {
				selected[selected_size++] = i;
			}
		}

		// Reproduction
		for (i32 i = 0; i < POPULATION_SIZE; i++) {
			i32 index = selected[random() % selected_size];

			Genome *individual = population + index;

			// Reproduce by cloning...
			population_buffer[i] = *individual;

			// ...with mutation
			Genome &child = population_buffer[i];
			i32 point = random() % child.num_choices;
			child.num_choices = point;
		}

		Genome *temp = population_buffer;
		population_buffer = population;
		population = temp;
	}

	for (i32 i = 0; i < POPULATION_SIZE; i++) {
		// Prepare grid
		memset(grid, EMPTY, DIM*DIM);

		// Print it out
		if (fitness[i] <= 10000) {
			printf("------------ %d ------------\n", fitness[i]);
			calculate_fitness(grid, population_buffer + i, true);
		}
	}

	return 0;
}
