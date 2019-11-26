#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include "mpi.h"

using namespace std;

int main(int argc, char** argv) {
    int rank, name_len;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(proc_name, &name_len);

	for (int i = 0; i < 3; i++) {
		if (rank == 0) {
			time_t cur_time = time(NULL);
			tm *now = localtime(&cur_time);
			printf("%d:%d.%d\n", now->tm_hour, now->tm_min, now->tm_sec);
		}

		system("sleep 56");
	}
    MPI_Finalize();

    return 0;
}
