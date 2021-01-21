#include <stdio.h>
#include<time.h>

int function(int n) {
	if (n <= 1) {
		return 1;
	}
	return function(n-1) + function(n-2);
}

int main() {
	clock_t start = clock();
	for (int i = 0; i < 10; i++) {
		function(40);
	}
	clock_t end = clock();
	printf("%d\n", end - start);
}