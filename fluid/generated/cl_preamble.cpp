#include "../buffer.cpp"
namespace cl_manager {
	struct HashBucket {
		char count;
		int particles[32];
	};
}