#include <cstdlib>
#include <cstdio>
#include <omp.h>

#define N 10
#define M 11

using namespace std;

int main(int argc, char** argv) {
	int b_array[N], s_array_l[M], s_array_wl[M];
	omp_lock_t lock;

	printf("b_arrray: ");
	for (int i = 0; i < N; i++) {
		b_array[i] = rand() % 2;
		printf("%d ", b_array[i]);
	}
	printf("\n");

	s_array_l[0] = s_array_wl[0] = 0;

	omp_init_lock(&lock);

	#pragma omp parallel for
	for (int i = 1; i < M; i++) {
		s_array_wl[i] = s_array_wl[i - 1] + b_array[i - 1];

		omp_set_lock(&lock);
		s_array_l[i] = s_array_l[i - 1] + b_array[i - 1];
		omp_unset_lock(&lock);
	}

	omp_destroy_lock(&lock);

	printf("s_arrray_l: ");
	for (int i = 0; i < M; i++) {
		printf("%d ", s_array_l[i]);
	}
	printf("\n");

	printf("s_arrray_wl: ");
	for (int i = 0; i < M; i++) {
		printf("%d ", s_array_wl[i]);
	}
	printf("\n");

	return 0;
}