// Copyright 2012 - Stefano Brilli : stefanobrilli@gmail.com
//
// Computes the Levenshtein distance between 2 files.
//
// Information about the Levenshtein distance: 
// http://en.wikipedia.org/wiki/Levenshtein_distance
//
// Other implementations
// http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance
//
// sample usage:
// $ ./lev_distance file1 file2
//
// Operations: O(m*n)
// Memory: O(max(m, n))
// where m and n are respectively the length of file1 and file2.
//

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <vector>

// Threads per block: since this is just an exercise, we use a fixed
// TPB size. In a real case the best configuration should be searched
// based on the underlying hardware

#define TPB 128

// A simple macro to check the return value from the CUDA calls.
// A better macro could be the CUDA_SAFE_CALL from the cuda SDK
#define CUDA_CHECK(function_call) if (function_call != cudaSuccess) \
{ fprintf(stderr, "ERR: %s\n", #function_call); exit(1); }

// Opens a file_name, allocates a buffer big as the content of the entire file
// and finally put the file content into the buffer.
//
// Returns the file size
// The ownership of the buffer is passed to the caller function.
// YOU MUST CALL free() ON RETURNED BUFFER
//
// No NULLs are not allowed as input
int LoadFileOrDie(const char* file_name, char** file_content) {
    FILE* fd;
    int size = 0;
    bool fail = (fd=fopen(file_name, "rb")) == NULL ||
        fseek(fd, 0, SEEK_END) < 0 ||
        (size = ftell(fd)) < 0 ||
        (*file_content=(char*)malloc(size)) == NULL ||
        fseek(fd, 0, SEEK_SET) != 0 ||
        int(fread(*file_content, 1, size, fd)) != size ||
        fclose(fd) == -1;
    if (fail) {
        fprintf(stderr, "Can't load file: %s\n", file_name);
        exit(1);
    }
    return size;
}

// Returns the Levenshtein distance to change file1_data into file2_data.
int LevDistance(char* file1_data, int file1_size,
             char* file2_data, int file2_size) {
    int sz = file2_size+1;
    std::vector<int> current(sz);
    std::vector<int> previous(sz);
    for (int i=0; i < sz; ++i) {
        previous[i] = i;
    }

    for (int i=0; i < file1_size; ++i) {
        current[0] = i+1;
        for (int j=1 ; j < sz; ++j) {
            current[j] = std::min( std::min(previous[j], current[j-1])+1, 
                previous[j-1]+(file1_data[i] != file2_data[j-1] ? 1 : 0));
        }
        std::swap(current, previous);
    }
    return previous[sz-1];
}

// Handles diagonals from 1 to m+1
__global__ void stage0(char* file1_data, char* file2_data, int iteration, 
                        int* curr, int* prev, int* prev2) {
    const int idx = blockDim.x * blockIdx.x + threadIdx.x;
    if (idx > iteration) {
        return;
    }
    if (idx == 0 || idx == iteration) {
        curr[idx] = iteration;
        return;
    }
    curr[idx] = min( min(prev[idx-1], prev[idx])+1, 
        prev2[idx-1] + int(file1_data[iteration-idx-1] != file2_data[idx-1]));
}

// Handles diagonals from m+2 to n+1
template <int ALIGN>
__global__ void stage1(char* file1_data, char* file2_data, int len, int iteration,
                        int* curr, int* prev, int* prev2) {
    const int idx = blockDim.x * blockIdx.x + threadIdx.x;
    if (idx > len) {
        return;
    }
    if (idx == len) {
        curr[len] = iteration+len+1;
        return;
    }
    curr[idx] = min( min(prev[idx], prev[idx+1])+1, 
        prev2[idx+ALIGN] + int(file1_data[len-idx-1] != file2_data[idx+iteration]));
}

// Handles diagonals from n+2 to n+m
template <int ALIGN>
__global__ void stage2(char* file1_data, char* file2_data, int len, int iteration, int gap,
                        int* curr, int* prev, int* prev2) {
    const int idx = blockDim.x * blockIdx.x + threadIdx.x;
    if (idx >= len-iteration) {
        return;
    }
    curr[idx] = min( min(prev[idx], prev[idx+1])+1, 
        prev2[idx+ALIGN] + int(file1_data[len-idx-1] != file2_data[iteration+idx+gap]));
}

// Returns the Levenshtein distance to change file1_data into file2_data.
// 
// WARNING: the function assumes that file2_size >= file1_size
//
// Information about this function can be found at http:://cybcode.blogspot.com
// inside the series of posts "A GPU Exercise"
int CudaLevDistance(char* file1_data, // X
             int file1_size, // 
             char* file2_data,
             int file2_size) {
    int sz = file1_size+1;
    int gap = file2_size-file1_size;

    // GPU pointers
    char* d_file1_data;
    char* d_file2_data;
    int* d_current;
    int* d_previous;
    int* d_previous2;

    // Allocates GPU memory
    CUDA_CHECK( cudaMalloc(&d_current, sz*sizeof(int)) ); // Current diagonal
    CUDA_CHECK( cudaMalloc(&d_previous, sz*sizeof(int)) ); // Previous diagonal
    CUDA_CHECK( cudaMalloc(&d_previous2, sz*sizeof(int)) ); // Previous of previous diagonal
    CUDA_CHECK( cudaMalloc(&d_file1_data, file1_size)); // file1_data on GPU
    CUDA_CHECK( cudaMalloc(&d_file2_data, file2_size)); // file2_data on GPU

    // Inits values
    CUDA_CHECK( cudaMemcpy(d_file1_data, file1_data, file1_size, cudaMemcpyHostToDevice));
    CUDA_CHECK( cudaMemcpy(d_file2_data, file2_data, file2_size, cudaMemcpyHostToDevice));
    CUDA_CHECK( cudaMemset(d_previous, 0, sizeof(int)*sz) );
    CUDA_CHECK( cudaMemset(d_current, 0, sizeof(int)*sz) );
    CUDA_CHECK( cudaMemset(d_previous2, 0, sizeof(int)*sz) );

    // Step 1: diagonals from 1 to m+1
    for (int i=1; i < file1_size+1; ++i) {
        stage0<<<(file1_size+TPB-1)/TPB+1, TPB, TPB*sizeof(int)>>>(d_file1_data, d_file2_data, i, 
            d_current, d_previous, d_previous2);
        std::swap(d_current, d_previous2);
        std::swap(d_previous2, d_previous);
    }

    // Step 2: diagonals from m+2 to n+1
    if ( gap > 0 ) { // Special case
        stage1 <0> <<<(file1_size+TPB-1)/TPB+1, TPB>>>(d_file1_data, d_file2_data,
            file1_size, 0, d_current, d_previous, d_previous2);
        std::swap(d_current, d_previous2);
        std::swap(d_previous2, d_previous);
    }
    for (int i=1; i < gap; ++i) {
        stage1 <1> <<<(file1_size+TPB-1)/TPB+1, TPB>>>(d_file1_data, d_file2_data,
            file1_size, i, d_current, d_previous, d_previous2);
        std::swap(d_current, d_previous2);
        std::swap(d_previous2, d_previous);
    }

    // Step 3: diagonals from n+2 to n+m
    if ( gap == 0 ) {
        stage2 <0> <<<(file1_size+TPB-1)/TPB+1, TPB>>>(d_file1_data, d_file2_data,
            file1_size, 0, gap, d_current, d_previous, d_previous2);
    } else {
        stage2 <1> <<<(file1_size+TPB-1)/TPB+1, TPB>>>(d_file1_data, d_file2_data,
            file1_size, 0, gap, d_current, d_previous, d_previous2);
    }
    std::swap(d_current, d_previous2);
    std::swap(d_previous2, d_previous);

    for (int i=1; i < file1_size; ++i) {
        stage2 <1> <<<(file1_size+TPB-1)/TPB+1, TPB>>>(d_file1_data, d_file2_data,
            file1_size, i, gap, d_current, d_previous, d_previous2);
        std::swap(d_current, d_previous2);
        std::swap(d_previous2, d_previous);
    }

    // Fetch the result from last diagonal (of lenght 1)
    int result = -1;
    cudaMemcpy(&result, d_previous, sizeof(int), cudaMemcpyDeviceToHost);

    cudaFree(d_current);
    cudaFree(d_previous);
    cudaFree(d_previous2);
    cudaFree(d_file1_data);
    cudaFree(d_file2_data);
    return result;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s file1 file2\n", argv[0]);
        return 1;
    }

    char* file1 = NULL;
    int file1_size = LoadFileOrDie(argv[1], &file1);

    char* file2 = NULL;
    int file2_size = LoadFileOrDie(argv[2], &file2);

    printf("%s size is %d bytes\n", argv[1], file1_size);
    printf("%s size is %d bytes\n", argv[2], file2_size);

    if (file1_size > file2_size) {
        std::swap(file1_size, file2_size);
        std::swap(file1, file2);
    }

    clock_t timer = clock();
    printf("LevDistance...\n");
    int ld = LevDistance(file1, file1_size, file2, file2_size);
    printf("elapsed time: %.3f (s)\n", double(clock()-timer)/CLOCKS_PER_SEC);

    timer = clock();
    printf("CudaLevDistance...\n");
    int cld = CudaLevDistance(file1, file1_size, file2, file2_size);
    printf("elapsed time: %.3f (s)\n", double(clock()-timer)/CLOCKS_PER_SEC);

    if (ld != cld) {
        fprintf(stderr, "CRITICAL: LevDistance result differs from CudaLevDistance!");
        exit(1);
    } else {
        printf("OK: CPU-GPU Result matches!\n");
    }

    printf("Distance: %d\n", cld);

    // No need to free memory since the program ends
    free(file1);
    free(file2);

    return 0;
}
