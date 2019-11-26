#include "mpich/mpi.h"

using namespace std;

int main(int argc, char** argv) {
    int rank, size, name_len;
    char proc_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm new_comm;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(proc_name, &name_len);

    int color;
    switch (rank % 3) {
    case 0:
        color = 0;
        break;
    case 1:
        color = 3;
        break;
    case 2:
        color = MPI_UNDEFINED; 
        break;
    }

    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &new_comm);
    if (new_comm == MPI_COMM_NULL) {
        printf("WORLD rank/size: %d/%d/%s \t NOT INCLUDED\n",
            rank, size, proc_name);
    }
    else {
        int new_rank, new_size;
        MPI_Comm_size(new_comm, &new_size);
        MPI_Comm_rank(new_comm, &new_rank);

        printf("WORLD rank/size: %d/%d/%s \t NEW_COMM rank/size: %d/%d\n",
             rank, size, proc_name, new_rank, new_size);
     
        MPI_Comm_free(&new_comm);
    }

    MPI_Finalize();

    return 0;
}
