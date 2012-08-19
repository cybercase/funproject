#include "debug_ptr.h"
#include <cstdlib>
#include <iostream>
#include <vector>

typedef_pointer(int*, int_p);
typedef_array(int_p*, int_pa);
typedef_array(int_pa*, int_paa, debug_types::PrintPolicy);

int main(int argc, char** argv) {

	const int N = 10;
	const int M = 10;
	int_paa other;
	int_paa matrix = new int_pa[M];
	for (int i = 0; i < M; ++i) {
		matrix[i] = new int_p[N];
		for (int j=0; j < N; ++j) {
			int_pa tmp = matrix[i];
			tmp[j] = new int(i*M+j);
		}
	}

	for (int i = 0; i < M; ++i) {
		for (int j=0; j < N; ++j) {
			delete matrix[i][j];
		}
		delete [] matrix[i];
	}
	// Try to comment the following line to see how this memory leak is handled
	other = matrix;
	delete [] other;
}
