#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void count_array_element_advanced_kernel(const int *input, int *count, int target, int N) {
    __shared__ int shared_count[kBlockSize];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + tid;
    // 先把命中结果转成 0/1，在 block 内归约后只发起一次 atomicAdd。
    shared_count[tid] = (idx < N && input[idx] == target) ? 1 : 0;
    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            shared_count[tid] += shared_count[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0 && shared_count[0] > 0) {
        atomicAdd(count, shared_count[0]);
    }
}

void launch_count_array_element_advanced(const int *input, int *count, int target, int N) {
    if (N <= 0) {
        return;
    }
    int blocks = (N + kBlockSize - 1) / kBlockSize;
    count_array_element_advanced_kernel<<<blocks, kBlockSize>>>(input, count, target, N);
}
