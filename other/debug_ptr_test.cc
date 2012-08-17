#define ENABLE_DEBUG_TYPES
#include "debug_ptr.h"
#include <cstdlib>
#include <iostream>
#include <vector>

typedef_array(int, int_pa);

int main(int argc, char** argv) {
	const int N = 10;
	const int M = 10;

	std::vector< int_pa > matrix;
	for (int i = 0; i < M; ++i) {
		matrix.push_back(new int[N]);
		for (int j=0; j < N; ++j) {
			int_pa tmp = matrix.back();
			tmp[j]=i*M+j;
		}
	}


	for (int i = 0; i < M; ++i)
		delete [] matrix[i];
}
