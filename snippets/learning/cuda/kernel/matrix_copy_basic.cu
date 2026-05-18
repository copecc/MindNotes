#include <cuda_runtime.h>

__global__ void matrix_copy_basic_kernel(const float *input, float *output, int width, int height) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (col < width && row < height) {
        int idx = row * width + col;
        output[idx] = input[idx];
    }
}

void launch_matrix_copy_basic(const float *input, float *output, int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((width + threads.x - 1) / threads.x,
                (height + threads.y - 1) / threads.y);
    matrix_copy_basic_kernel<<<blocks, threads>>>(input, output, width, height);
}
