#include <cuda_runtime.h>

__global__ void silu_basic_kernel(const float *input, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        float x = input[idx];
        output[idx] = x / (1.0f + __expf(-x));
    }
}

void launch_silu_basic(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    silu_basic_kernel<<<blocks, threads>>>(input, output, N);
}
