#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <omp.h>

using namespace std;

#define INITIAL_TEMP 24
#define HEAT_TEMP 500

void print_dmatrix(double **matrix_arr, int &matrix_size) {
	for (int i = 0; i < matrix_size; i++) {
		for (int j = 0; j < matrix_size; j++) {
			printf("%7.3f ", matrix_arr[i][j]);
		}
		printf("\n");
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("#Error# Only size of grid required!\n");
		return -1;
	}

	int grid_size = atoi(argv[1]);

	double **grid_arr = new double*[grid_size];
	double **grid_arr_temp = new double*[grid_size];
	for (int i = 0; i < grid_size; i++) {
		grid_arr[i] = new double[grid_size];
		grid_arr_temp[i] = new double[grid_size];
	}

	for (int i = 0; i < grid_size; i++) {
		for (int j = 0; j < grid_size; j++) {
			if (i == 0 || i == grid_size - 1 || j == 0 || j == grid_size - 1) {
				grid_arr[i][j] = HEAT_TEMP;
				grid_arr_temp[i][j] = HEAT_TEMP;
			}
			else {
				grid_arr[i][j] = INITIAL_TEMP;
			}
		}
	}

	printf("Grid before calculation:\n");
	print_dmatrix(grid_arr, grid_size);

	double max_change, epsil = 20, temp, d, dm;
	int i;

	omp_lock_t lock;
	omp_init_lock(&lock);

	do {
		max_change = 0;

		#pragma omp parallel for shared(grid_arr, grid_arr_temp, grid_size, max_change) private(i, d, dm)
		for (i = 1; i < grid_size - 1; i++) {
			dm = 0;
			for (int j = 1; j < grid_size - 1; j++) {
				grid_arr_temp[i][j] = 0.25 * (grid_arr[i - 1][j] + grid_arr[i + 1][j] + grid_arr[i][j - 1] + grid_arr[i][j + 1]);
				d = fabs(grid_arr[i][j] - grid_arr_temp[i][j]);
				
				if (dm < d) {
					dm = d;
				}

				omp_set_lock(&lock);
				if (max_change < dm) {
					max_change = dm;
				}
				omp_unset_lock(&lock);
			}
		}

		for (i = 0; i < grid_size; i++) {
			for (int j = 0; j < grid_size; j++) {
				grid_arr[i][j] = grid_arr_temp[i][j];
			}
		}

	} while (max_change > epsil);

	omp_destroy_lock(&lock);

	printf("Grid after calculation:\n");
	print_dmatrix(grid_arr, grid_size);

	for (int i = 0; i < grid_size; i++) {
		delete[] grid_arr[i];
		delete[] grid_arr_temp[i];
	}

	delete[] grid_arr;
	delete[] grid_arr_temp;

	return 0;
}
