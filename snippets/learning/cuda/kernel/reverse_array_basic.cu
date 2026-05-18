#include <cuda_runtime.h>

__global__ void reverse_array_basic_kernel(const float *input, float *output, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[N - 1 - idx] = input[idx];
    }
}

void launch_reverse_array_basic(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    reverse_array_basic_kernel<<<blocks, threads>>>(input, output, N);
}
