#include <omp.h>
#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv) {
	int i, j;

	#pragma omp parallel for shared(j)
	for (i = 0; i < 100; i++)
		j = i;

	printf("j = %d\n", j);

	return 0;
}
