/// Wrap os threading
#if defined(OS_WINDOWS)
	#include <process.h>
	struct Win32Thread {
		HANDLE handle;
		DWORD id;
	};
	typedef Win32Thread ThreadType;

	typedef u32(__stdcall *ThreadStartRoutine)(void *arg);
	#define THREAD_FUNC_RETURN_TYPE u32

	inline bool create_thread(ThreadStartRoutine start_routine, void *arg, ThreadType *out_thread, u32 stack_size) {
		HANDLE handle = reinterpret_cast<HANDLE>(_beginthreadex(0, stack_size, start_routine, arg, CREATE_SUSPENDED, 0));

		if (handle == 0) {
			return false;
		}

		out_thread->handle = handle;
		out_thread->id = GetThreadId(handle);
		ResumeThread(handle);

		return true;
	}

	// Terminate the current thread
	inline void exit_thread() {
		_endthreadex(0);
	}

	// Blocking until 'thread' finishes
	inline void join_thread(ThreadType thread) {
		WaitForSingleObject(thread.handle, INFINITE);
	}

	// Get the current thread
	inline ThreadType get_current_thread() {
		Win32Thread result = {
			::GetCurrentThread(),
			::GetCurrentThreadId()
		};

		return result;
	}

	// Get the number of hardware threads. This should take Hyperthreading, etc. into account
	inline int get_num_hardware_threads() {
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	}

	inline b32 threads_equal(ThreadType thread_a, ThreadType thread_b) {
		return thread_a.id == thread_b.id;
	}
	#define THREAD_LOCAL __declspec(thread)
#elif defined(OS_MAC) || defined(OS_iOS) || defined(OS_LINUX)
	#include <pthread.h>
	#include <unistd.h>

	typedef pthread_t ThreadType;

	typedef void *(*ThreadStartRoutine)(void *arg);
	#define THREAD_FUNC_RETURN_TYPE void *

	inline bool create_thread(ThreadStartRoutine start_routine, void *arg, ThreadType *out_thread, u32 stack_size = (u32)-1) {
		pthread_attr_t thread_attr;
		pthread_attr_init(&thread_attr);

		// Set stack size
		if (stack_size != (u32)-1) {
			pthread_attr_setstacksize(&thread_attr, stack_size);
		}
        // pthread_attr_setaffinity_np(&thread_attr, 0, 0);
		int success = pthread_create(out_thread, NULL, start_routine, arg);
		// printf("Thread: %lu\n", (uintptr_t)*out_thread);
		pthread_attr_destroy(&thread_attr);

		return success == 0;
	}

	__attribute__((noreturn))
	inline void exit_thread() {
		pthread_exit(NULL);
	}

	// Blocking until 'thread' finishes
	inline void join_thread(ThreadType thread) {
		pthread_join(thread, NULL);
	}

	// Get the current thread
	inline ThreadType get_current_thread() {
		return pthread_self();
	}

	// Get the number of hardware threads. This should take Hyperthreading, etc. into account
	inline int get_num_hardware_threads() {
		return (int) sysconf(_SC_NPROCESSORS_ONLN);
	}
	inline b32 threads_equal(ThreadType thread_a, ThreadType thread_b) {
		return pthread_equal(thread_a, thread_b);
	}

	#define THREAD_LOCAL thread_local
#endif
