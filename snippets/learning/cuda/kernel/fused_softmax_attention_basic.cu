#include <cuda_runtime.h>
#include <cfloat>
#include <cmath>

constexpr int kBlockSize = 256;
constexpr unsigned kFullMask = 0xffffffffu;

__device__ float attention_warp_reduce_max(float value) {
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 16));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 8));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 4));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 2));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 1));
    return value;
}

__device__ float attention_warp_reduce_sum(float value) {
    value += __shfl_down_sync(kFullMask, value, 16);
    value += __shfl_down_sync(kFullMask, value, 8);
    value += __shfl_down_sync(kFullMask, value, 4);
    value += __shfl_down_sync(kFullMask, value, 2);
    value += __shfl_down_sync(kFullMask, value, 1);
    return value;
}

__device__ float attention_block_reduce_max(float value) {
    __shared__ float warp_values[kBlockSize / 32];
    int lane = threadIdx.x & 31;
    int warp_id = threadIdx.x >> 5;

    value = attention_warp_reduce_max(value);
    if (lane == 0) {
        warp_values[warp_id] = value;
    }
    __syncthreads();

    int warp_count = (blockDim.x + 31) / 32;
    value = (threadIdx.x < warp_count) ? warp_values[lane] : -FLT_MAX;
    if (warp_id == 0) {
        value = attention_warp_reduce_max(value);
    }
    return value;
}

__device__ float attention_block_reduce_sum(float value) {
    __shared__ float warp_values[kBlockSize / 32];
    int lane = threadIdx.x & 31;
    int warp_id = threadIdx.x >> 5;

    value = attention_warp_reduce_sum(value);
    if (lane == 0) {
        warp_values[warp_id] = value;
    }
    __syncthreads();

    int warp_count = (blockDim.x + 31) / 32;
    value = (threadIdx.x < warp_count) ? warp_values[lane] : 0.0f;
    if (warp_id == 0) {
        value = attention_warp_reduce_sum(value);
    }
    return value;
}

__global__ void fused_softmax_attention_basic_kernel(const float *__restrict__ q,
                                                     const float *__restrict__ k,
                                                     const float *__restrict__ v,
                                                     float *__restrict__ out,
                                                     int seq_len,
                                                     int head_dim,
                                                     float scale) {
    int q_idx = blockIdx.x;
    int d = blockIdx.y;
    if (q_idx >= seq_len || d >= head_dim) {
        return;
    }

    float local_max = -FLT_MAX;
    for (int k_idx = threadIdx.x; k_idx < seq_len; k_idx += blockDim.x) {
        float score = 0.0f;
        for (int i = 0; i < head_dim; ++i) {
            score += q[q_idx * head_dim + i] * k[k_idx * head_dim + i];
        }
        local_max = fmaxf(local_max, score * scale);
    }
    float max_value = attention_block_reduce_max(local_max);

    __shared__ float shared_max;
    if (threadIdx.x == 0) {
        shared_max = max_value;
    }
    __syncthreads();

    float local_sum = 0.0f;
    float local_out = 0.0f;
    for (int k_idx = threadIdx.x; k_idx < seq_len; k_idx += blockDim.x) {
        float score = 0.0f;
        for (int i = 0; i < head_dim; ++i) {
            score += q[q_idx * head_dim + i] * k[k_idx * head_dim + i];
        }
        float weight = expf(score * scale - shared_max);
        local_sum += weight;
        local_out += weight * v[k_idx * head_dim + d];
    }

    float sum_value = attention_block_reduce_sum(local_sum);
    float out_value = attention_block_reduce_sum(local_out);

    __shared__ float shared_sum;
    if (threadIdx.x == 0) {
        shared_sum = sum_value;
    }
    __syncthreads();

    if (threadIdx.x == 0) {
        out[q_idx * head_dim + d] = out_value / shared_sum;
    }
}

void launch_fused_softmax_attention_basic(const float *q,
                                          const float *k,
                                          const float *v,
                                          float *out,
                                          int seq_len,
                                          int head_dim,
                                          float scale) {
    if (seq_len <= 0 || head_dim <= 0) {
        return;
    }

    dim3 blocks(seq_len, head_dim);
    fused_softmax_attention_basic_kernel<<<blocks, kBlockSize>>>(q, k, v, out, seq_len, head_dim, scale);
}
