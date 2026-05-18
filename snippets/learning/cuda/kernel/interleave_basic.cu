#include <cuda_runtime.h>

__global__ void interleave_basic_kernel(const float *a, const float *b, float *output, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[2 * idx] = a[idx];
        output[2 * idx + 1] = b[idx];
    }
}

void launch_interleave_basic(const float *a, const float *b, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    interleave_basic_kernel<<<blocks, threads>>>(a, b, output, N);
}
