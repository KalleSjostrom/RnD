#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include "engine/utils/common.h"
#include "engine/utils/file_utils.h"
#include "engine/utils/memory/memory_arena.cpp"
#include "engine/utils/string.h"
#include "utils/file_system.cpp"
#include "engine/utils/containers/dynamic_array.h"
#include "engine/utils/containers/hashmap.cpp"
#include "engine/utils/serialize.h"
#include "utils/generation_cache.cpp"
#include "utils/parser.cpp"

#define FIBER_STACK_GUARD_PAGES
#include "engine/utils/fibers/task_scheduler.cpp"
