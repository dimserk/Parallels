#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <omp.h>

#define N 10
#define M 10

using namespace std;

int main(int argc, char** argv) {
	omp_set_num_threads(3);

	// Master
	printf("--- Master section ---\n");
	#pragma omp parallel
	{
		printf("Proc#%d before master section\n", omp_get_thread_num());
		#pragma omp master
		{
			printf("Proc#%d inside master section\n", omp_get_thread_num());
		}
	}

	// Critical
	printf("--- Critical section ---\n");

	int x = 0;

	printf("X=%d before parallel section\n", x);
	#pragma omp parallel
	{
		printf("Proc#%d before critical section\n", omp_get_thread_num());
		#pragma omp critical
		{
			printf("Proc#%d inside critical section, incrimenting X\n", omp_get_thread_num());
			x++;
		}
	}
	printf("X=%d after parallel section\n", x);

	// Barrier
	printf("--- Barrier ---\n");
	#pragma omp parallel
	{
		printf("Proc#%d before barrier\n", omp_get_thread_num());
		#pragma omp barrier
		printf("Proc#%d after barrier\n", omp_get_thread_num());
	}

	// Atomic
	printf("--- Atomic ---\n");

	int y = 0;

	printf("Y array befor parallel for: %d\n", y);

	#pragma omp parallel
	{
		#pragma omp atomic
		y++;
	}

	printf("Y array after parallel for: %d\n", y);

	//// Flush
	printf("--- Flush ---\n");

	int z = 0;

	#pragma omp parallel sections shared(z)
	{
		#pragma omp section
		{
			z = 1;
			printf("Proc#%d flushing\n", omp_get_thread_num());
			#pragma omp flush
		}

		#pragma omp section
		{
			printf("Proc#%d inside while\n", omp_get_thread_num());
			while (!z);
		}
	}

	// Ordered
	printf("--- Ordered ---\n");

	#pragma omp parallel for ordered 
	for (int k = 0; k < 5; k++) {
		sleep(1 + rand() % 2);
		printf("No order: %d\n", k);
		
		#pragma omp ordered
		printf("Order: %d\n", k);
	}

	return 0;
}
