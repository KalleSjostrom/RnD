//// ARRAY GROWING ////
void array_setup(ArrayTest &test) {
	for (int i = 0; i < ARRAY_TEST_SIZE; ++i) {
		test.a[i] = i;
	}
	test.end = 0xDEADBEEF;
}
void array_reloaded(ArrayTest &test) {
	for (int i = 0; i < ARRAY_TEST_SIZE; ++i) {
		ASSERT(test.a[i] == i, "Array");
	}
	ASSERT(test.end == 0xDEADBEEF, "End");
}

void tests_setup(Application &app) {
	array_setup(app.array_test);
}
void tests_reloaded(Application &app) {
	array_reloaded(app.array_test);
}