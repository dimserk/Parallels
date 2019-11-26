#include <omp.h>
#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv) {
	int i, s = 0;

	#pragma onp parallel for reduction(+:s)
	for (i = 0; i < 100; i++)
		s += i;

	printf("Sum: %d\n", s);

	return 0;
}
