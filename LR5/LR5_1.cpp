#include <omp.h>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("#Error# Only one argument allowed!\n");
		return 0;
	}

	int proc_num = atoi(argv[1]);
	omp_set_num_threads(proc_num);
	printf("Num of threads set to %d\n", proc_num);

	printf("Single block\n");
	#pragma omp single
	{
		int proc_id = omp_get_thread_num();
		printf("Proc#%d\n", proc_id);
	}

	printf("Parallel block with master section\n");
	#pragma omp parallel
	{
		int proc_id = omp_get_thread_num();
		printf("Proc#%d parallel section\n", proc_id);
		#pragma omp master
		{
			printf("Proc#%d master section\n", proc_id);
		}
	}

	printf("Parallel block with nowait section begins\n");
	#pragma omp parallel
	{
		int tmp, proc_id = omp_get_thread_num();
		#pragma omp single nowait
		{
			printf("Proc#%d into single section\n", proc_id);
			scanf("%d", &tmp);
		}
		printf("Proc#%d after single section\n", proc_id);
	}
	printf("Parallel block with nowait section ends\n");

	return 0;
}