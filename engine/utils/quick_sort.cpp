struct SortElement {
	u32 value;
	u32 index;
};

inline u32 calculate_pivot(SortElement *list, u32 size) {
	u64 a = (u64)list[0].value;
	u64 b = (u64)list[size-1].value;
	u64 average = (a + b)/2;
	return (u32)average;
}

void quick_sort(SortElement *list, u32 size) {
	if (size <= 1)
		return;

	u32 pivot = calculate_pivot(list, size);

	i32 right = (i32)size;
	i32 left = -1;

	while (1) {
		do {
			right--;
		} while ((list + right)->value > pivot);
		do {
			left++;
		} while ((list + left)->value < pivot);

		if (left < right) {
			SortElement temp = list[left];
			list[left] = list[right];
			list[right] = temp;
		} else {
			break;
		}
	}
	quick_sort(list, (u32)left);
	quick_sort(list+right+1, (size-((u32)right+1)));
}
