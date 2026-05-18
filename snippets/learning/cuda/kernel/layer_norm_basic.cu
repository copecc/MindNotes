#include <cuda_runtime.h>
#include <stdio.h>

#define WARP_SIZE 32

// Warp级规约求和
__inline__ __device__ float warp_reduce_sum(float val) {
#pragma unroll
    for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
    }
    return val;
}

// Block级规约求和
__inline__ __device__ float block_reduce_sum(float val) {
    static __shared__ float shared[32]; // 最多支持 1024 个线程 (32个Warp)
    int lane = threadIdx.x % WARP_SIZE;
    int wid = threadIdx.x / WARP_SIZE;

    // Warp内规约
    val = warp_reduce_sum(val);

    // 将每个Warp的规约结果写入Shared Memory
    if (lane == 0) {
        shared[wid] = val;
    }

    __syncthreads();

    // 读取Shared Memory的内容并进行最后的Warp内规约
    int warp_count = (blockDim.x + WARP_SIZE - 1) / WARP_SIZE;
    val = (threadIdx.x < warp_count) ? shared[lane] : 0.0f;
    if (wid == 0) {
        val = warp_reduce_sum(val);
    }
    return val;
}

// Layer Normalization 基础内核
// 特征形状: [batch_size, hidden_size]
// blockIdx.x 处理 batch_size 中的一行
// threadIdx.x 分担隐层维度的求和修正
__global__ void layer_norm_basic(const float* __restrict__ x, 
                                 const float* __restrict__ gamma, 
                                 const float* __restrict__ beta, 
                                 float* __restrict__ y, 
                                 int hidden_size, 
                                 float epsilon) {
    int row = blockIdx.x;
    int tid = threadIdx.x;
    
    // 定位到当前处理的数据行
    const float* row_x = x + row * hidden_size;
    float* row_y = y + row * hidden_size;

    // 1. 计算当前行的均值
    float local_sum = 0.0f;
    for (int i = tid; i < hidden_size; i += blockDim.x) {
        local_sum += row_x[i];
    }
    float sum = block_reduce_sum(local_sum);
    
    __shared__ float s_mean;
    if (tid == 0) {
        s_mean = sum / hidden_size;
    }
    __syncthreads();
    float mean = s_mean;

    // 2. 计算当前行的方差
    float local_variance = 0.0f;
    for (int i = tid; i < hidden_size; i += blockDim.x) {
        float diff = row_x[i] - mean;
        local_variance += diff * diff;
    }
    float var_sum = block_reduce_sum(local_variance);
    
    __shared__ float s_variance;
    if (tid == 0) {
        s_variance = var_sum / hidden_size;
    }
    __syncthreads();
    float variance = s_variance;

    // 3. 执行归一化与缩放平移
    float inv_std = rsqrtf(variance + epsilon);
    for (int i = tid; i < hidden_size; i += blockDim.x) {
        float norm_val = (row_x[i] - mean) * inv_std;
        
        // 可选：应用放射变换 (Affine Transform)
        if (gamma != nullptr && beta != nullptr) {
            row_y[i] = norm_val * gamma[i] + beta[i];
        } else {
            row_y[i] = norm_val;
        }
    }
}
