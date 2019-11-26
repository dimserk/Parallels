#include <iostream>
#include "mpich/mpi.h"

using namespace std;

int main(int argc, char** argv) {
    int world_rank, world_size;

    int *send_buf, *recv_buf;

    int small_recv_buf;
    int *red_scat_count;

    MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_size < 2) {
        cout << "#Error# Not enought processes!" << endl;
        goto error;
    }

    send_buf = new int[world_size];
    recv_buf = new int[world_size];

    // ***Reduce***
    cout << "Proc#" << world_rank << " reduce(SUM) array: [ ";
    for (int i = 0; i < world_size; i++) {
        send_buf[i] = 10 * (i + 1) + world_rank;
        cout << send_buf[i] << " ";
    }
    cout << "]" << endl;

	MPI_Barrier(MPI_COMM_WORLD);

    MPI_Reduce(send_buf, recv_buf, world_size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        cout << "Proc#0 reduced: [ ";
        for (int i = 0; i < world_size; i++)
            cout << recv_buf[i] << " ";
        cout << "]" << endl;
    }

	MPI_Barrier(MPI_COMM_WORLD);

    // ***Allreduce***
    cout << "Proc#" << world_rank << " allreduce(PROD) array: [ ";
    for (int i = 0; i < world_size; i++) {
        send_buf[i] = (world_rank + 1) * 10 + i;
        cout << send_buf[i] << " ";
    }
    cout << "]" << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Allreduce(send_buf, recv_buf, world_size, MPI_INT, MPI_PROD, MPI_COMM_WORLD);

    cout << "Proc#" << world_rank << " allreduced: [ ";
    for (int i = 0; i < world_size; i++) 
        cout << recv_buf[i] << " ";
    cout << "]" << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    // ***Reduce_scatter***
    red_scat_count = new int[world_size];

    cout << "Proc#" << world_rank << " allreduce_scatter(MAX) array: [ ";
    for (int i = 0; i < world_size; i++) {
        send_buf[i] = (world_rank + 1) * 10;
        red_scat_count[i] = 1;

        cout << send_buf[i] << " ";
    }
    cout << "]" << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Reduce_scatter(send_buf, &small_recv_buf, red_scat_count, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    cout << "Proc#" << world_rank << " allreduce_scattered: " << small_recv_buf << endl;

    delete[] red_scat_count;

    MPI_Barrier(MPI_COMM_WORLD);

    // ***Scan***
    cout << "Proc#" << world_rank << " scan(SUM) array: [ ";
    for (int i = 0; i < world_size; i++) {
        send_buf[i] = world_rank + 1;
        cout << send_buf[i] << " ";
    }
    cout << "]" << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Scan(send_buf, recv_buf, world_size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    cout << "Proc#" << world_rank << " scaned: [ ";
    for (int i = 0; i < world_size; i++)
        cout << recv_buf[i] << " ";
    cout << "]" << endl;

    delete[] send_buf;
    delete[] recv_buf;

error:
    MPI_Finalize();

    return 0;
}
