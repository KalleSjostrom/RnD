
struct Stopwatch {
	LARGE_INTEGER frequency;
	LARGE_INTEGER start_counter;

	Stopwatch() {
		QueryPerformanceFrequency(&frequency);
		start();
	}
	void start() {
		QueryPerformanceCounter(&start_counter);
	}
	double stop() {
		LARGE_INTEGER stop_counter;
		QueryPerformanceCounter(&stop_counter);
		return (double)(stop_counter.QuadPart - start_counter.QuadPart) / frequency.QuadPart;
	}
};
