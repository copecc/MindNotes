#include <cuda_runtime.h>

__global__ void histogram_basic_kernel(const int* input, int* bins, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx < N) {
        int bin_idx = input[idx];
        // 输入值必须落在合法 bin 范围内，否则会越界写 bins。
        if (bin_idx >= 0 && bin_idx < NUM_BINS) {
            atomicAdd(&bins[bin_idx], 1);
        }
    }
}
