#pragma warning(disable : 4255)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4710) // function not inlined

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <assert.h>

#include "utils/array.h"
#include "utils/hashmap.h"

#include "memory/memory.c"

#include "fibers/task_scheduler.c"

typedef struct {
	int test;
} Compiler;

void run(TaskScheduler *scheduler, void *args) {

}

int main(int argc, char *argv[]) {
#if 0
	char *project_path = 0;

	for (int i = 0; i < argc; ++i) {
		char *option = argv[i];
		if (strcmp(option, "--project") == 0) {
			project_path = argv[++i];
		}
	}

	assert(project_path && "Need to specify a project file to compile! Use --project path/to/some_project");
	printf("Running compiler for project '%s'\n", project_path);
	fflush(stdout); // We want to se the status update when running from sublime..

	Compiler compiler = {};

	compiler.context.arena = &compiler.arena;
	compiler.context.file_system = &compiler.file_system;

	parse_project(compiler.arena, compiler.project, project_path);
#endif

	Compiler compiler;
	scheduler_start(run, &compiler);
	return 0;
}
