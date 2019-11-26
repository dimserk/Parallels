#include "mpich/mpi.h"

#define TAG 10

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        printf("#Error# Only two processes allowed!\n");
        goto error;
    }

    if (rank == 0) {
        int send_num = -1;

        MPI_Send(&send_num, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
        printf("Proc #%d - data sended\n", rank);
    }
    else if (rank == 1) {
        int recv_num;

        MPI_Recv(&recv_num, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Proc #%d - data recived\n", rank);
        printf("Recived message: %d\n", recv_num);
    }
    
error:
    MPI_Finalize();
    
    return 0;
}
