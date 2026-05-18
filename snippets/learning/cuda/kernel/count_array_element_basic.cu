#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void count_array_element_basic_kernel(const int *input, int *count, int target, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N && input[idx] == target) {
        atomicAdd(count, 1);
    }
}

void launch_count_array_element_basic(const int *input, int *count, int target, int N) {
    if (N <= 0) {
        return;
    }
    int blocks = (N + kBlockSize - 1) / kBlockSize;
    count_array_element_basic_kernel<<<blocks, kBlockSize>>>(input, count, target, N);
}
