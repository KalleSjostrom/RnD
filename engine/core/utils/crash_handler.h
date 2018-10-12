#include <windows.h>
#include <dbghelp.h>

void print_callstack(int line, const char *file, const char *assert, const char *msg, unsigned skip_frames = 0);
LONG WINAPI exception_filter(EXCEPTION_POINTERS *ep);
void exceptions_setup();
void exceptions_shutdown();
