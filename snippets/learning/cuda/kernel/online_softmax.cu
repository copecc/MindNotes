#include <cuda_runtime.h>
#include <float.h>
#include <math.h>

#define WARP_SIZE 32

// Warp 级别的 Online Softmax 规约
__inline__ __device__ void warp_reduce_online_softmax(float& local_max, float& local_sum) {
#pragma unroll
    for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
        float other_max = __shfl_down_sync(0xffffffff, local_max, offset);
        float other_sum = __shfl_down_sync(0xffffffff, local_sum, offset);
        
        float new_max = fmaxf(local_max, other_max);
        float exp_local = expf(local_max - new_max);
        float exp_other = expf(other_max - new_max);
        
        local_sum = local_sum * exp_local + other_sum * exp_other;
        local_max = new_max;
    }
}

// Block 级别的 Online Softmax 规约
__inline__ __device__ void block_reduce_online_softmax(float& local_max, float& local_sum) {
    static __shared__ float s_max[32];
    static __shared__ float s_sum[32];

    int lane = threadIdx.x % WARP_SIZE;
    int wid = threadIdx.x / WARP_SIZE;

    warp_reduce_online_softmax(local_max, local_sum);

    if (lane == 0) {
        s_max[wid] = local_max;
        s_sum[wid] = local_sum;
    }
    __syncthreads();

    float other_max = (threadIdx.x < blockDim.x / WARP_SIZE) ? s_max[lane] : -FLT_MAX;
    float other_sum = (threadIdx.x < blockDim.x / WARP_SIZE) ? s_sum[lane] : 0.0f;

    if (wid == 0) {
        warp_reduce_online_softmax(other_max, other_sum);
    }
    
    // 广播最终的全局 max 和 sum 到所有 block 线程
    if (threadIdx.x == 0) {
        s_max[0] = other_max;
        s_sum[0] = other_sum;
    }
    __syncthreads();
    
    local_max = s_max[0];
    local_sum = s_sum[0];
}

// Online Softmax 内核
// 采用一趟（单重循环归约计算特征统计量）
// input/output: [batch_size, seq_len]
__global__ void online_softmax_kernel(const float* __restrict__ input, 
                                      float* __restrict__ output, 
                                      int seq_len) {
    int row = blockIdx.x;
    int tid = threadIdx.x;
    
    const float* row_in = input + row * seq_len;
    float* row_out = output + row * seq_len;

    // 1. 在读取过程中复用寄存器维护局部的在线 max 和 sum
    float thread_max = -FLT_MAX;
    float thread_sum = 0.0f;

    for (int i = tid; i < seq_len; i += blockDim.x) {
        float val = row_in[i];
        float max_new = fmaxf(thread_max, val);
        float sum_new = thread_sum * expf(thread_max - max_new) + expf(val - max_new);
        thread_max = max_new;
        thread_sum = sum_new;
    }

    // 2. Block 内合并在线统计量
    float block_max = thread_max;
    float block_sum = thread_sum;
    block_reduce_online_softmax(block_max, block_sum);

    // 3. 最后计算出最终激活输出
    for (int i = tid; i < seq_len; i += blockDim.x) {
        float val = row_in[i];
        row_out[i] = expf(val - block_max) / block_sum;
    }
}
