#include <omp.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "#Error# Only one argument allowed!" << endl;
		return 0;
	}

	int proc_num = atoi(argv[1]);
	omp_set_num_threads(proc_num);
	cout << "Num of threads set to " << proc_num << endl;

	#pragma omp parallel sections
	{
		#pragma omp section 
		{
			//cout << "Proc#" << omp_get_thread_num() << " inside first(HARD CODED) section" << endl;
			printf("Proc#%d inside first(HARD CODED) section\n", omp_get_thread_num());
		}

		#pragma omp section 
		{
			//cout << "Proc#" << omp_get_thread_num() << " inside second(HARD CODED) section" << endl;
			printf("Proc#%d inside second(HARD CODED) section\n", omp_get_thread_num());
		}

		#pragma omp section
		{
			//cout << "Proc#" << omp_get_thread_num() << " inside third(HARD CODED) section" << endl;
			printf("Proc#%d inside third(HARD CODED) section\n", omp_get_thread_num());
		}
	}

	return 0;
}