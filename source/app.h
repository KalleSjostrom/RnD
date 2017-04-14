#include "../utils/common.h"

#define EXPORT extern "C" __attribute__((visibility("default")))

#define APP_UPDATE(name) int name(void *memory, float dt)
typedef APP_UPDATE(app_update_t);

#define APP_RELOAD(name) void name(void *old_memory, void *new_memory)
typedef APP_RELOAD(app_reload_t);