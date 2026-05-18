#include <cuda_runtime.h>

__global__ void vector_addition_basic_kernel(const float *a, const float *b, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = a[idx] + b[idx];
    }
}

void launch_vector_addition_basic(const float *a, const float *b, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    vector_addition_basic_kernel<<<blocks, threads>>>(a, b, output, N);
}
