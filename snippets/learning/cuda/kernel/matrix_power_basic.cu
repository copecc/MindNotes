#include <cuda_runtime.h>

constexpr int kBlockSize = 256;
constexpr int kThreadOutputs = 4;
constexpr int kMaxKernelSize = 4096;
__constant__ float c_kernel[kMaxKernelSize];

__global__ void matrix_power_basic_kernel(const float *left,
                                         const float *right,
                                         float *output,
                                         int n) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    if (row >= n || col >= n) {
        return;
    }

    float sum = 0.0f;
    for (int k = 0; k < n; ++k) {
        sum += left[row * n + k] * right[k * n + col];
    }
    output[row * n + col] = sum;
}

void launch_matrix_power_basic(const float *left,
                               const float *right,
                               float *output,
                               int n) {
    if (n <= 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((n + threads.x - 1) / threads.x,
                (n + threads.y - 1) / threads.y);
    matrix_power_basic_kernel<<<blocks, threads>>>(left, right, output, n);
}
