/////////// Unsigned
struct SortElement {
	unsigned value;
	unsigned index;
};

__forceinline unsigned calculate_pivot(SortElement *list, unsigned size) {
	uint64_t a = (uint64_t)list[0].value;
	uint64_t b = (uint64_t)list[size-1].value;
	uint64_t average = (a + b)/2;
	return (unsigned)average;
}

void quick_sort(SortElement *list, unsigned size) {
	if (size <= 1)
		return;

	unsigned pivot = calculate_pivot(list, size);

	int right = (int)size;
	int left = -1;

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
	quick_sort(list, (unsigned)left);
	quick_sort(list+right+1, (size-((unsigned)right+1)));
}

/////////// Float
struct SortElementf {
	float value;
	unsigned index;
};
__forceinline float calculate_pivot(SortElementf *list, unsigned size) {
	double a = (double)list[0].value;
	double b = (double)list[size - 1].value;
	double c = 2.0;
	double average = (a + b) / c;
	return (float)average;
}
void quick_sort(SortElementf *list, unsigned size) {
	if (size <= 1)
		return;

	float pivot = calculate_pivot(list, size);

	int right = size;
	int left = -1;

	while (1) {
		do {
			right--;
		} while ((list + right)->value > pivot);
		do {
			left++;
		} while ((list + left)->value < pivot);

		if (left < right) {
			SortElementf temp = list[left];
			list[left] = list[right];
			list[right] = temp;
		}
		else {
			break;
		}
	}
	quick_sort(list, left);
	quick_sort(list + right + 1, size - (right + 1));
}
