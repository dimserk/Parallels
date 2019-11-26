#include "mpich/mpi.h"

#define TAG 10

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 1) {
        printf("#Error# Only one proc, he has no friend to talk with!\n");
        goto error;
    }

    for (int id = 0; id < size; id++) {
        if (rank == id) {
            for (int dest = 0; dest < size; dest++) {
                if (dest == rank) continue;
                MPI_Send(&rank, 1, MPI_INT, dest, TAG, MPI_COMM_WORLD);
            }
            printf("Proc #%d - data sended\n", rank);
        }
        else {
            int recv_num;
            MPI_Recv(&recv_num, 1, MPI_INT, id, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Proc #%d - data recived from %d\n", rank, recv_num);
        }
    }

error:
    MPI_Finalize();
    
    return 0;
}
