#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void count_array_element2d_advanced_kernel(const int *input, int *count, int rows, int cols, int target) {
    // 采用 Shared Memory 在 Block 内局部累加
    __shared__ int shared_count[kBlockSize];

    // 将 2D thread 索引平铺为 1D 以访问 shared block
    int tid = threadIdx.y * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    
    // 初始化 shared memory，对越界或者不匹配的数据记 0
    shared_count[tid] = (row < rows && col < cols && input[row * cols + col] == target) ? 1 : 0;
    __syncthreads();

    // Block 内部对局部计数进行树状归约 (Parallel Reduction)
    for (int stride = kBlockSize / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            shared_count[tid] += shared_count[tid + stride];
        }
        __syncthreads();
    }

    // 由0号线程将 Block 的计算总结果合并进全局计数器
    if (tid == 0 && shared_count[0] > 0) {
        atomicAdd(count, shared_count[0]);
    }
}

void launch_count_array_element2d_advanced(const int *input, int *count, int rows, int cols, int target) {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((cols + threads.x - 1) / threads.x,
                (rows + threads.y - 1) / threads.y);
    count_array_element2d_advanced_kernel<<<blocks, threads>>>(input, count, rows, cols, target);
}
