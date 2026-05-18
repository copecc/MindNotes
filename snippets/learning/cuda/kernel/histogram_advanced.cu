#include <cuda_runtime.h>

// 假设我们有静态确定的 Bin 数量，这里设为 256
constexpr int NUM_BINS = 256;

__global__ void histogram_advanced_kernel(const int* input, int* bins, int N) {
    // 使用 Shared Memory 在 Block 内建立私有的局部直方图，提升数据复用
    __shared__ int s_bins[NUM_BINS];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    // 前 N 个线程负责初始化共享内存
    if (tid < NUM_BINS) {
        s_bins[tid] = 0;
    }
    __syncthreads(); // 同步块内所有线程，确保初始化完成

    // 局部直方图累加（冲突被限制在 Shared Memory 层级）
    if (idx < N) {
        int bin_idx = input[idx];
        if (bin_idx >= 0 && bin_idx < NUM_BINS) {
            atomicAdd(&s_bins[bin_idx], 1);
        }
    }
    __syncthreads(); // 同步块内所有线程，确保所有像素计算完毕

    // 最终将局部直方图合并到全局直方图
    if (tid < NUM_BINS) {
        atomicAdd(&bins[tid], s_bins[tid]);
    }
}
