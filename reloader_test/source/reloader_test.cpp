#define PLUGIN_DATA ReloadTest

struct Data {
	int a;
	int b;
};

// TODO(kalle): We need to handle the case where multiple pointers point to it though, with different types (like void* and SomeStruct*).
// Case 1: Data and Pointers
// Case 2: Arrays w. Pointers
// Case 3: MultiDim Arrays w. Pointers
// Case 4: Functions (post-poned)
// Case 5: Several different mspace allocations
// Case 6: Pointing to outside memory

struct ReloadTest {
#if defined(CASE_1_A)
	int a;
#elif defined(CASE_1_B)
	int b;
	int a;
#elif defined(CASE_1_C)
	int a;
	int *b;
	int c;
#elif defined(CASE_1_D)
	int a;
	int c;
	int *b;
#endif

#if defined(CASE_2_A)
	int a[8];
#elif defined(CASE_2_B)
	int b[8];
	int a[8];
#elif defined(CASE_2_C)
	int a[8];
	int *b[8];
	int c[8];
#elif defined(CASE_2_D)
	int a[8];
	int c[8];
	int *b[8];
#endif

#if defined(CASE_3_A)
	int a[8][8];
#elif defined(CASE_3_B)
	int b[8][8];
	int a[8][8];
#elif defined(CASE_3_C)
	int a[8][8];
	int *b[8][8];
	int c[8][8];
#elif defined(CASE_3_D)
	int a[8][8];
	int c[8][8];
	int *b[8][8];
#endif

#if defined(CASE_4_A)
	Data *a[8];
	Data storage[8];
#elif defined(CASE_4_B)
	int b[8];
	Data storage[8];
	Data *a[8];
#endif

#if defined(CASE_5_A)
	void (*a)(void);
#elif defined(CASE_5_B)
	void (*b)(int, int);
	void (*a)(void);
#elif defined(CASE_5_C)
	void (*a)(void);
	void (*b)(int, int);
#endif
};

#define FEATURE_RELOAD
#include "systems.h"

void plugin_update(Application *application, float dt) {
	ReloadTest &test = application->plugin_data;
}

void plugin_render(Application *application) {
	ReloadTest &test = application->plugin_data;
}

void plugin_setup(Application *application) {
	ReloadTest &test = application->plugin_data;

#if defined(CASE_1_A)
	test.a = 5;
#elif defined(CASE_1_B)
	test.a = 3;
	test.b = 8;
#elif defined(CASE_1_C)
	test.a = 3;
	test.b = &test.a;
	test.c = 12;
#endif

#if defined(CASE_2_A)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		test.a[i] = 500 + i;
	}
#elif defined(CASE_2_B)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		test.a[i] = 300 + i;
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		test.b[i] = 800 + i;
	}
#elif defined(CASE_2_C)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		test.a[i] = 300 + i;
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		test.b[i] = &test.a[i];
	}
	for (int i = 0; i < ARRAY_COUNT(test.c); ++i) {
		test.c[i] = 1200 + i;
	}
#endif

#if defined(CASE_3_A)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			test.a[i][j] = 500 + i * 100 + j;
		}
	}
#elif defined(CASE_3_B)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			test.a[i][j] = 300 + i * 100 + j;
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			test.b[i][j] = 800 + i * 100 + j;
		}
	}
#elif defined(CASE_3_C)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			test.a[i][j] = 300 + i * 100 + j;
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			test.b[i][j] = &test.a[i][j];
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.c); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			test.c[i][j] = 1200 + i * 100 + j;
		}
	}
#endif

#if defined(CASE_4_A)
	for (int i = ARRAY_COUNT(test.storage) - 1; i >= 0; i--) {
		test.a[i] = &test.storage[i];
	}
#endif

#if defined(CASE_5_A)
	test.a = &testa;
#elif defined(CASE_5_B)
	test.a = &testa;
	test.b = &testb;
#elif defined(CASE_5_C)
	test.a = &testa;
	test.b = &testb;
#endif
}

#define TEST(condition) do { \
	log_info("Reloader", "Testing " #condition ## "."); \
	if (condition) { \
		log_info("Reloader", "Success!"); \
	} else { \
		log_info("Reloader", "Failed!"); \
	} \
} while(0)

void plugin_reloaded(Application *application) {
	ReloadTest &test = application->plugin_data;

#if defined(CASE_1_B)
	TEST(test.a == 5);
	TEST(test.b == 0);
#elif defined(CASE_1_C)
	TEST(test.a == 3);
	TEST(test.b == 0);
	TEST(test.c == 0);
#elif defined(CASE_1_D)
	TEST(test.a == 3);
	TEST(test.b == &test.a);
	TEST(*test.b == 3);
	TEST(test.c == 12);
#endif

#if defined(CASE_2_B)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		TEST(test.a[i] == 500 + i);
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		TEST(test.b[i] == 0);
	}
#elif defined(CASE_2_C)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		TEST(test.a[i] == 300 + i);
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		TEST(test.b[i] == 0);
	}
#elif defined(CASE_2_D)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		TEST(test.a[i] == 300 + i);
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		TEST(test.b[i] == &test.a[i]);
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		TEST(*test.b[i] == 300 + i);
	}
	for (int i = 0; i < ARRAY_COUNT(test.c); ++i) {
		TEST(test.c[i] == 1200 + i);
	}
#endif

#if defined(CASE_3_B)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			TEST(test.a[i][j] == 500 + i * 100 + j);
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.b[i]); ++j) {
			TEST(test.b[i][j] == 0);
		}
	}
#elif defined(CASE_3_C)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			TEST(test.a[i][j] == 300 + i * 100 + j);
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.b[i]); ++j) {
			TEST(test.b[i][j] == 0);
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.c); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.c[i]); ++j) {
			TEST(test.c[i][j] == 0);
		}
	}
#elif defined(CASE_3_D)
	for (int i = 0; i < ARRAY_COUNT(test.a); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			TEST(test.a[i][j] == 300 + i * 100 + j);
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.b); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.a[i]); ++j) {
			TEST(test.b[i][j] == &test.a[i][j]);
		}
	}
	for (int i = 0; i < ARRAY_COUNT(test.c); ++i) {
		for (int j = 0; j < ARRAY_COUNT(test.c[i]); ++j) {
			TEST(test.c[i][j] == 1200 + i * 100 + j);
		}
	}
#endif

#if defined(CASE_4_B)
	for (int i = ARRAY_COUNT(test.storage) - 1; i >= 0; i--) {
		TEST(test.a[i] == &test.storage[i]);
	}
#endif

#if defined(CASE_5_B)
	TEST(test.a == &testa);
	TEST(test.b == 0);
#elif defined(CASE_5_C)
	TEST(test.a == &testa);
	TEST(test.b == &testb);
#endif

	plugin_setup(application);
}