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
    static __shared__ float shared[32]; 
    int lane = threadIdx.x % WARP_SIZE;
    int wid = threadIdx.x / WARP_SIZE;

    val = warp_reduce_sum(val);
    if (lane == 0) shared[wid] = val;
    __syncthreads();

    int warp_count = (blockDim.x + WARP_SIZE - 1) / WARP_SIZE;
    val = (threadIdx.x < warp_count) ? shared[lane] : 0.0f;
    if (wid == 0) val = warp_reduce_sum(val);
    return val;
}

// Batch Normalization 2D (NCHW format) 基础内核
// 特征形状: [N, C, H, W]
// blockIdx.x 负责一个独占的通道 (Channel), 即网格大小对应维度 C
// threadIdx.x 负责处理跨 Batch 和空间维度 (N * H * W)
__global__ void batch_norm_2d_basic(const float* __restrict__ x, 
                                    const float* __restrict__ scale, 
                                    const float* __restrict__ bias, 
                                    float* __restrict__ y, 
                                    int N, int C, int H, int W, 
                                    float epsilon) {
    int c = blockIdx.x; // 当前通道
    int tid = threadIdx.x;
    
    int spatial_size = H * W;
    int channel_stride = C * spatial_size; // 一个样本的跨度
    int num_elements = N * spatial_size;   // 该通道总共需要处理的数据量
    
    // 1. 求当前通道在整个 Batch 下的均值
    float local_sum = 0.0f;
    for (int i = tid; i < num_elements; i += blockDim.x) {
        int n = i / spatial_size;
        int hw = i % spatial_size;
        // 映射到 NCHW 内存布局的实际索引
        int idx = n * channel_stride + c * spatial_size + hw;
        local_sum += x[idx];
    }
    float sum = block_reduce_sum(local_sum);
    
    __shared__ float s_mean;
    if (tid == 0) {
        s_mean = sum / num_elements;
    }
    __syncthreads();
    float mean = s_mean;

    // 2. 求当前通道的方差
    float local_var = 0.0f;
    for (int i = tid; i < num_elements; i += blockDim.x) {
        int n = i / spatial_size;
        int hw = i % spatial_size;
        int idx = n * channel_stride + c * spatial_size + hw;
        float diff = x[idx] - mean;
        local_var += diff * diff;
    }
    float var_sum = block_reduce_sum(local_var);
    
    __shared__ float s_variance;
    if (tid == 0) {
        s_variance = var_sum / num_elements;
    }
    __syncthreads();
    float variance = s_variance;

    // 3. 执行归一化与缩放平移
    float inv_std = rsqrtf(variance + epsilon);
    float ch_scale = scale != nullptr ? scale[c] : 1.0f;
    float ch_bias = bias != nullptr ? bias[c] : 0.0f;

    for (int i = tid; i < num_elements; i += blockDim.x) {
        int n = i / spatial_size;
        int hw = i % spatial_size;
        int idx = n * channel_stride + c * spatial_size + hw;
        
        float norm_val = (x[idx] - mean) * inv_std;
        y[idx] = norm_val * ch_scale + ch_bias;
    }
}
