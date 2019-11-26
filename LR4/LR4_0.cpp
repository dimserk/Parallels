#include "mpich/mpi.h"

int main(int argc, char **argv) {
	int world_rank, world_size;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//Broadcast vars
	int broadcast_num;

	//Gather vars
	int *gather_array = new int[world_size];

	//Scatter vars
	int scatter_num;
	int *scatter_array = new int[world_size];
	if (world_rank == 0) {
		for (int i = 0; i < world_size; i++) {
			scatter_array[i] = i + 30;
		}
	}

	//Allgather vars
	int *allgather_array = new int[world_size];

	//All_to_all
	int *alltoall_self_array = new int[world_size];
	int *recv_buf = new int[world_size]; 
		for (int i = 0; i < world_size; i++) {
		alltoall_self_array[i] = (world_rank * 10) + i;
	}


	if (world_size < 2) {
		printf("#Error# Not enought processes!\n");
		goto error;
	}


	// ***Broadcasting***
	if (world_rank == 0) {
		broadcast_num = -123;
		printf("Proc#0 broadcast sendind ( %d )\n", broadcast_num);
	}

	MPI_Bcast(&broadcast_num, 1, MPI_INT, 0, MPI_COMM_WORLD);
	printf("Proc#%d broadcast recived: %d\n", world_rank, broadcast_num);

	MPI_Barrier(MPI_COMM_WORLD);


	// ***Gathering***
	MPI_Gather(&world_rank, 1, MPI_INT, gather_array, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	if (world_rank == 0) {
		printf("Proc#0 gathering data: [ ");
		for (int i = 0; i < world_size; i++) {
			printf("%d ", gather_array[i]);
		}
		printf("]\n");
	}
	
	printf("Proc#%d gathered: %d\n", world_rank, world_rank);

	MPI_Barrier(MPI_COMM_WORLD);


	// ***Scattering****
	MPI_Scatter(scatter_array, 1, MPI_INT, &scatter_num, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (world_rank == 0) {
		printf("Proc#0 scattering [ ");
		for (int i = 0; i < world_size; i++) {
			printf("%d ", scatter_array[i]);
		}
		printf("]\n");
	}
	
	printf("Proc#%d scaterred: %d\n", world_rank, scatter_num);

	MPI_Barrier(MPI_COMM_WORLD);


	// ***Allgather****
	MPI_Allgather(&world_rank, 1, MPI_INT, allgather_array, 1, MPI_INT, MPI_COMM_WORLD);

	printf("Proc#%d all_gathering [ ", world_rank);
	for (int i = 0; i < world_size; i++) {
		printf("%d ", allgather_array[i]);
	}
	printf("]\n");

	MPI_Barrier(MPI_COMM_WORLD);


	// ***Alltoall***
	printf("Proc#%d all_to_all before: [ ", world_rank);
	for (int i = 0; i < world_size; i++) {
		printf("%d ", alltoall_self_array[i]);
	}
	printf("]\n");

	MPI_Alltoall(alltoall_self_array, 1, MPI_INT, recv_buf, 1, MPI_INT, MPI_COMM_WORLD);

	printf("Proc#%d all_to_all after: [ ", world_rank);
	for (int i = 0; i < world_size; i++) {
		printf("%d ", recv_buf[i]);
	}
	printf("]\n");

error:
	MPI_Finalize();

	delete[] gather_array;
	delete[] scatter_array;
	delete[] allgather_array;
	delete[] alltoall_self_array;
	delete[] recv_buf;

	return 0;
}
