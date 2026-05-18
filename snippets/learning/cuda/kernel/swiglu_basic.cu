#include <cuda_runtime.h>

__global__ void swiglu_basic_kernel(const float *value, const float *gate, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = value[idx] / (1.0f + __expf(-gate[idx]));
    }
}

void launch_swiglu_basic(const float *value, const float *gate, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    swiglu_basic_kernel<<<blocks, threads>>>(value, gate, output, N);
}
