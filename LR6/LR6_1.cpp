#include <omp.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("#Error# Only one arguments allowed!\n");
		return -1;
	}

	int shcedule_num = atoi(argv[1]);
	printf("Shcedule number set to %d\n", shcedule_num);

	omp_set_num_threads(4);

	#pragma omp parallel 
	{
		#pragma omp for schedule(static, shcedule_num)
		for (int i = 0; i < 10; i++) {
			printf("Static %d: One more for iteration (Proc#%d)\n", i + 1, omp_get_thread_num());
		}

		#pragma omp for schedule(dynamic, shcedule_num)
		for (int i = 0; i < 10; i++) {
			printf("Dynamic %d: One more for iteration (Proc#%d)\n", i + 1, omp_get_thread_num());
		}
	}

	return 0;
}
