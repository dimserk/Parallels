#include <cstdlib>
#include <cstdio>
#include <omp.h>

using namespace std;

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("#Error# Only one argument allowed - vector`s length!\n");
		return -1;
	}

	int v_len = atoi(argv[1]);

	int *v_arr1 = new int[v_len];
	int *v_arr2 = new int[v_len];
	int *v_arr3 = new int[v_len];

	for (int i = 0; i < v_len; i++) {
		v_arr1[i] = 1 + rand() % 10;
		v_arr2[i] = 1 + rand() % 10;
	}

	printf("Vector1: ");
	for (int i = 0; i < v_len; i++) {
		printf("%d ", v_arr1[i]);
	}
	printf("\n");

	printf("Vector2: ");
	for (int i = 0; i < v_len; i++) {
		printf("%d ", v_arr2[i]);
	}
	printf("\n");

	omp_set_num_threads(v_len);
	#pragma omp parallel
	{
		int id = omp_get_thread_num();
		v_arr3[id] = v_arr1[id] + v_arr2[id];
	}

	printf("Vector3: ");
	for (int i = 0; i < v_len; i++) {
		printf("%d ", v_arr3[i]);
	}
	printf("\n");

	delete[] v_arr1;
	delete[] v_arr2;
	delete[] v_arr3;

	return 0;
}