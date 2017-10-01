#include <stdio.h>

int get_number() {
	int number=0;
	while (1) {
		char c = getchar();

		if (c == '\n' || c == ' ') {
			break;
		}

		int	n = c-'0';
		if (n>9 || n<0) {
			printf("error non valid value\n");
		} else {
			number=number*10+n;
		}
	}
	return number;
}

int main(int argc, char const *argv[]) {
	/*float n;
	scanf("%f", &n);*/

	int result = 0;
	int a = get_number();
	char operand = getchar();
	char space = getchar();
	if (space != ' ') {
		printf("error space required!\n");
	}

	int b = get_number();
	if (operand == '+') {
		result = a + b;
	} else if (operand == '-') {
		result = a - b;
	} else if (operand == '*') {
		result = a * b;
	}

	printf("Result is: %d\n", result);

	return 0;
}
