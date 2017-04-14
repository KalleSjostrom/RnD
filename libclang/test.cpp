#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// //@ Short Comment
// struct List {
// 	__attribute__((annotate("event: test")))
// 	void* data;
// };

struct A {
	//! Serialize
	int integer1;
	int integer2;
	//! Serialize Export Editor(Slider Min:0 Max:10)
	int integer3;
};

int main(int argc, char** argv) {
	return 0;
}
