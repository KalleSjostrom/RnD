#include <string.h>
#include "engine/utils/common.h"
#include "engine/utils/profiler.c"

/*
sysctl -a | grep machdep.cpu

machdep.cpu.cache.linesize: 64
machdep.cpu.cache.L2_associativity: 8
machdep.cpu.cache.size: 256

  L2 Cache (per Core):	256 KB
  L3 Cache:	6 MB
*/

enum ProfilerScopes {
	ProfilerScopes__sequential_vector,
	ProfilerScopes__sequential_vector_inner,
	ProfilerScopes__sequential_scalar,
	ProfilerScopes__sequential_scalar_inner,
	ProfilerScopes__pthread_scalar,
	ProfilerScopes__pthread_scalar_inner,
	ProfilerScopes__pthread_vector,
	ProfilerScopes__pthread_vector_inner,
	ProfilerScopes__write_ppm,

	ProfilerScopes__count,
};

#include "engine/utils/math/vectorization.h"

#define WIDTH 10240
#define HEIGHT 7680
#define SIZE (WIDTH * HEIGHT)
#define MAX_IT 60
#define THRESHOLD 2000 * 2000
#define X_MIN -2.0f
#define X_MAX 1.0f
#define Y_MIN -1.0f
#define Y_MAX 1.0f

#define X_IT_VAL ((X_MAX - (X_MIN)) / WIDTH)
#define Y_IT_VAL ((Y_MAX - (Y_MIN)) / HEIGHT)

#define TWO55_OVER_MAX_IT (255.0f / MAX_IT)

#if IACA
#include <iacaMarks.h>
#else
#define IACA_START
#define IACA_END
#endif

void sequential_vector(f32 *colors) {
	vec xmin = v_set(X_MIN);
	vec x_it = v_set(X_IT_VAL);

	vec zero = v_set(0.0f);
	vec two = v_set(2.0f);

	vec two55_over_max_it = v_set(TWO55_OVER_MAX_IT);
	vec threshold = v_set((f32)THRESHOLD);

	vec max_it = v_set(MAX_IT);
	vec inc = v_mul(x_it, v_set(VECTOR_WIDTH));
	vec vector_indices = VECTOR_INDICES;
	vector_indices = v_mul(x_it, vector_indices);

	PROFILER_START(sequential_vector_inner);
	for (i32 y = 0; y < HEIGHT; ++y) {
		vec cImag = v_set(Y_MIN + Y_IT_VAL * y);
		i32 ywidth =  y * WIDTH;
		vec x_x4 = vector_indices;
		for (i32 x = 0; x < WIDTH; x += VECTOR_WIDTH) {
			IACA_START;
			x_x4 = v_add(x_x4, inc);
			vec cReal = v_add(xmin, x_x4);

			vec real = zero;
			vec imaginary = zero;
			vec indices = zero;

			vec cmp_result = zero;

			i32 iterations = MAX_IT - 1;
			do {
				vec r2 = v_mul(real, real);
				vec i2 = v_mul(imaginary, imaginary);

				vec val = v_add(r2, i2);
				cmp_result = v_cmp_gt(val, threshold);

				vec current = v_set(iterations);
				indices = v_max(v_and(current, cmp_result), indices);

				vec two_imag = v_mul(imaginary, two);
				imaginary = v_mul(two_imag, real);
				imaginary = v_add(imaginary, cImag);

				real = v_sub(r2, i2);
				real = v_add(real, cReal);

				iterations--;
			} while (iterations >= 0 && v_movemask(cmp_result) != (1<<VECTOR_WIDTH)-1);

			cmp_result = v_cmp_gt(indices, zero);
			indices = v_sub(max_it, indices);
			indices = v_and(indices, cmp_result);
			i32 index = ywidth + x;
			v_store(colors + index, v_mul(indices, two55_over_max_it));
		}
		IACA_END;
	}
	PROFILER_STOP_HITS(sequential_vector_inner, HEIGHT * (WIDTH/VECTOR_WIDTH));
}

void sequential_scalar(f32 *colors) {
	PROFILER_START(sequential_scalar_inner);
	for (i32 y = 0; y < HEIGHT; y++) {
		i32 ywidth = y * WIDTH;
		f32 cImag = Y_MIN + Y_IT_VAL * y;
		for (i32 x = 0; x < WIDTH; x++) {
			f32 cReal = X_MIN + X_IT_VAL * x;

			f32 real = 0;
			f32 imaginary = 0;
			i32 i = 0;
			for (; i < MAX_IT; ++i) {
				f32 r2 = real * real;
				f32 i2 = imaginary * imaginary;

				if (r2+i2 > THRESHOLD)
					break;

				// a*a + 2abi + (bi)*(bi) --- bi^2 = -b*b
				imaginary = 2 * imaginary * real + cImag;
				real = r2-i2 + cReal;
			}

			if (i < MAX_IT) {
				i32 index = ywidth + x;
				colors[index] = TWO55_OVER_MAX_IT * (i-1);
			}
		}
	}
	PROFILER_STOP_HITS(sequential_scalar_inner, HEIGHT * WIDTH);
}

#define PTHREAD_TASK(name) void* name(void *job)
typedef PTHREAD_TASK(pthread_task_t);

typedef struct {
	f32 *colors;
	i32 index;
	i32 __padding;
} Job;

#define NUMBER_OF_THREADS 64
#define LAST_THREAD_INDEX (NUMBER_OF_THREADS - 1)
#include <pthread.h>

PTHREAD_TASK(pthread_vector_task) {
	Job j = *(Job*) job;
	f32 *colors = j.colors;

	i32 h = HEIGHT / NUMBER_OF_THREADS;
	i32 start_height = j.index * h;
	i32 end_height = start_height + h;

	vec xmin = v_set(X_MIN);
	vec x_it = v_set(X_IT_VAL);

	vec zero = v_set(0.0f);
	vec two = v_set(2.0f);
	vec vector_indices = VECTOR_INDICES;

	vec two55_over_max_it = v_set(TWO55_OVER_MAX_IT);
	vec threshold = v_set((f32)THRESHOLD);

	for (i32 y = start_height; y < end_height; ++y) {
		vec cImag = v_set(Y_MIN + Y_IT_VAL * y);
		i32 ywidth =  y * WIDTH;
		for (i32 x = 0; x < WIDTH; x += VECTOR_WIDTH) {
			vec x_x4 = v_add(v_set(x), vector_indices);
			vec cReal = v_add(xmin, v_mul(x_it, x_x4));

			vec real = zero;
			vec imaginary = zero;
			vec indices = zero;

			for (i32 i = 0; i < MAX_IT; ++i) {
				vec r2 = v_mul(real, real);
				vec i2 = v_mul(imaginary, imaginary);

				vec val = v_add(r2, i2);
				vec cmp_result = v_cmp_gt(val, threshold);

				vec current = v_set(MAX_IT - i - 1);
				indices = v_max(v_and(current, cmp_result), indices);

				i32 mask = v_movemask(cmp_result);
				if (mask == (1<<VECTOR_WIDTH)-1) {
					break;
				}

				imaginary = v_mul(v_mul(imaginary, two), real);
				imaginary = v_add(imaginary, cImag);

				real = v_sub(r2, i2);
				real = v_add(real, cReal);
			}

			vec cmp_result = v_cmp_gt(indices, zero);
			indices = v_sub(v_set(MAX_IT), indices);
			indices = v_and(indices, cmp_result);
			i32 index = ywidth + x;
			v_store(colors + index, v_mul(indices, two55_over_max_it));
		}
	}
	return NULL;
}

PTHREAD_TASK(pthread_scalar_task) {
	Job j = *(Job*) job;

	f32 *colors = j.colors;

	i32 height = HEIGHT / NUMBER_OF_THREADS;
	i32 start_height = j.index * height;
	i32 end_height = start_height + height;

	for (i32 y = start_height; y < end_height; y++) {
		i32 ywidth = y * WIDTH;
		f32 cImag = Y_MIN + Y_IT_VAL * y;
		for (i32 x = 0; x < WIDTH; x++) {
			f32 cReal = X_MIN + X_IT_VAL * x;

			f32 real = 0;
			f32 imaginary = 0;
			i32 i = 0;
			for (; i < MAX_IT; ++i) {
				f32 r2 = real * real;
				f32 i2 = imaginary * imaginary;

				if (r2+i2 > THRESHOLD)
					break;

				imaginary = 2 * imaginary * real + cImag;
				real = r2-i2 + cReal;
			}

			i32 index = ywidth + x;
			colors[index] = TWO55_OVER_MAX_IT * (i-1);
		}
	}
	return NULL;
}
void run_pthread_task(pthread_task_t *task, f32* colors) {
	pthread_t *threads = (pthread_t*) calloc(NUMBER_OF_THREADS, sizeof(pthread_t));

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	Job jobs[NUMBER_OF_THREADS] = {};

	for (i32 i = 0; i < LAST_THREAD_INDEX; i++) {
		jobs[i].colors = colors;
		jobs[i].index = i;
		if (pthread_create(&threads[i], &attr, task, (void *) (jobs + i))) {
			fprintf(stderr, "Failed to create a thread\n");
		}
	}
	jobs[LAST_THREAD_INDEX].colors = colors;
	jobs[LAST_THREAD_INDEX].index = LAST_THREAD_INDEX;
	task((void *) (jobs + LAST_THREAD_INDEX));

	pthread_attr_destroy(&attr);
	for (i32 i = 0; i < LAST_THREAD_INDEX; i++) {
		if (pthread_join(threads[i], NULL)) {
			fprintf(stderr, "Failed to join a thread\n");
		}
	}

	free(threads);
}

void write_ppm(f32 *colors, const char *filename) {
	FILE *file = fopen(filename, "w");
	fprintf(file, "P6\n");
	fprintf(file, "%i %i\n", WIDTH, HEIGHT);
	fprintf(file, "255\n");

	u8 *buf = (u8 *) calloc(SIZE*3, sizeof(u8));
	for (i32 i = 0; i < SIZE; i++) {
		buf[i*3] = (u8)colors[i]; // We only calculate the red channel
	}
 	fwrite(buf, 1, SIZE*3, file);
	fclose(file);
}

i32 main() {
	f32 *colors;
	posix_memalign((void*)&colors, VECTOR_WIDTH * sizeof(f32), SIZE * sizeof(f32));


	TIME_IT(sequential_vector, sequential_vector(colors));
	PROFILER_PRINT(sequential_vector_inner);
	write_ppm(colors, "mm_fract.ppm");


	TIME_IT(sequential_scalar, sequential_scalar(colors));
	PROFILER_PRINT(sequential_scalar_inner);
	write_ppm(colors, "fract.ppm");


	TIME_IT(pthread_scalar, run_pthread_task(pthread_scalar_task, colors));
	write_ppm(colors, "pthread_fract.ppm");


	TIME_IT(pthread_vector, run_pthread_task(pthread_vector_task, colors));
	TIME_IT(write_ppm, write_ppm(colors, "pthread_mm_fract.ppm"));


	free(colors);
}
