struct WaitFreeQueue {
	int *array;
	int bottom;
	int top;
};

#define MAX_ARRAY_SIZE 1024
#define ARRAY_MASK (MAX_ARRAY_SIZE-1)

b32 queue_take(WaitFreeQueue *q, int &value) {
	int b = __atomic_load_n(&q->bottom, __ATOMIC_RELAXED) - 1;
	int *a = __atomic_load_n(&q->array, __ATOMIC_RELAXED);
	__atomic_store_n(&q->bottom, b, __ATOMIC_RELAXED);

	__atomic_thread_fence(__ATOMIC_SEQ_CST);

	int t = __atomic_load_n(&q->top, __ATOMIC_RELAXED);
	b32 result = 1;
	if (t <= b) {
		// Non-empty queue.
		value = __atomic_load_n(&a[b & ARRAY_MASK], __ATOMIC_RELAXED);
		if (t == b) {
			/* Single last element in queue. */
			if (!__atomic_compare_exchange_n(&q->top, &t, t + 1, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
				/* Failed race. */
				result = false;
			}
			__atomic_store_n(&q->bottom, b + 1, __ATOMIC_RELAXED);
		}
	} else { /* Empty queue. */
		result = false;
		__atomic_store_n(&q->bottom, b + 1, __ATOMIC_RELAXED);
	}
	return result;
}

void queue_push(WaitFreeQueue *q, int x) {
	int b = __atomic_load_n(&q->bottom, __ATOMIC_RELAXED);
	int t = __atomic_load_n(&q->top, __ATOMIC_ACQUIRE);
	int *a = __atomic_load_n(&q->array, __ATOMIC_RELAXED);
	if (b - t > MAX_ARRAY_SIZE - 1) { /* Full queue. */
		// resize(q);
		ASSERT(false, "Queue full!");
		// a = __atomic_load_n(&q->array, __ATOMIC_RELAXED);
	}
	__atomic_store_n(&a[b & ARRAY_MASK], x, __ATOMIC_RELAXED);
	__atomic_thread_fence(__ATOMIC_RELEASE);
	__atomic_store_n(&q->bottom, b + 1, __ATOMIC_RELAXED);
}

b32 queue_steal(WaitFreeQueue *q, int &value) {
	int t = __atomic_load_n(&q->top, __ATOMIC_ACQUIRE);
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	int b = __atomic_load_n(&q->bottom, __ATOMIC_ACQUIRE);
	if (t < b) {
		// Non-empty queue.
		int *a = __atomic_load_n(&q->array, __ATOMIC_CONSUME);
		value = __atomic_load_n(&a[t & ARRAY_MASK], __ATOMIC_RELAXED);
		// (type *ptr, type *expected, type desired, bool weak, int success_memorder, int failure_memorder)
		if (!__atomic_compare_exchange_n(&q->top, &t, t + 1, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
			// Failed race.
			return false;
		}
        return true;
	}
	return false;
}
