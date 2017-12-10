struct Profile {
	Profile(const char *name) {
		_Profiler.start(name);
	}
	~Profile() {
		_Profiler.stop();
	}
};