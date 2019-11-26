#include <omp.h>
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "#Error# Only one argument allowed!" << endl;
		return 0;
	}

	int proc_num = atoi(argv[1]);

	omp_set_num_threads(proc_num);
	#pragma omp parallel 
	{
		printf("Hello wolrd!\n");
	}

	return 0;
}