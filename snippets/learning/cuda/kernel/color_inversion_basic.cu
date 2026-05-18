#include <cuda_runtime.h>

__global__ void color_inversion_basic_kernel(const unsigned char *input, unsigned char *output, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[idx] = static_cast<unsigned char>(255 - input[idx]);
    }
}

void launch_color_inversion_basic(const unsigned char *input, unsigned char *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    color_inversion_basic_kernel<<<blocks, threads>>>(input, output, N);
}
