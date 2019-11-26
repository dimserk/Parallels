#include <omp.h>
#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv) {
	int num = 9;

	printf("Num before parallel is %d\n", num);

	omp_set_num_threads(4);
	#pragma omp parallel private(num)
	{
		num = omp_get_thread_num();
		printf("Proc#%d Num is %d\n", num, num);
	}

	printf("Num after private parallel is %d\n", num);

	printf("Num before firstprivate parallel is %d\n", num);
	#pragma omp parallel for firstprivate(num)
	for (int i = 0; i < 10; i++)
		printf("Proc#%d Num is %d\n", omp_get_thread_num(), num);


	return 0;
}
