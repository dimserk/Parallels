#include <omp.h>
#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("#Error# Only one arguments allowed!\n");
		return -1;
	}

	int iters_num = atoi(argv[1]);
	printf("For iterations set to %d\n", iters_num);

	#pragma omp for
	for (int i = 0; i < iters_num; i++) {
		printf("%d: One more for iteration\n", i + 1);
	}

	return 0;
}
