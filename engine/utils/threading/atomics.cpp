#if defined(OS_WINDOWS)
#include <atomic>
#define ai32 std::atomic_int
#define abool std::atomic_bool

#define __ATOMIC_RELAXED std::memory_order_relaxed
#define __ATOMIC_CONSUME std::memory_order_consume
#define __ATOMIC_ACQUIRE std::memory_order_acquire
#define __ATOMIC_RELEASE std::memory_order_release
#define __ATOMIC_ACQ_REL std::memory_order_acq_rel
#define __ATOMIC_SEQ_CST std::memory_order_seq_cst

#define __atomic_load_n(ptr, order) std::atomic_load_explicit(ptr, order)
#define __atomic_store_n(ptr, value, order) std::atomic_store_explicit(ptr, value, order)
#define __atomic_fetch_sub(ptr, value, order) std::atomic_fetch_sub_explicit(ptr, value, order)
#define __atomic_thread_fence(order) std::atomic_thread_fence(order)
#define __atomic_compare_exchange_weak_n(ptr, expected, desired, success_memorder, failure_memorder) std::atomic_compare_exchange_weak_explicit(ptr, expected, desired, success_memorder, failure_memorder)
#define __atomic_compare_exchange_strong_n(ptr, expected, desired, success_memorder, failure_memorder) std::atomic_compare_exchange_strong_explicit(ptr, expected, desired, success_memorder, failure_memorder)
#elif defined(OS_MAC) || defined(OS_iOS) || defined(OS_LINUX)
#define ai32 i32
#define abool bool
#define __atomic_compare_exchange_weak_n(ptr, expected, desired, success_memorder, failure_memorder) __atomic_compare_exchange_n(ptr, expected, desired, true, success_memorder, failure_memorder)
#define __atomic_compare_exchange_strong_n(ptr, expected, desired, success_memorder, failure_memorder) __atomic_compare_exchange_n(ptr, expected, desired, false, success_memorder, failure_memorder)
#endif
