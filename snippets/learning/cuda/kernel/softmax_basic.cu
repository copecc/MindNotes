#include <cuda_runtime.h>
#include <cfloat>
#include <cmath>

constexpr int kBlockSize = 256;
constexpr unsigned kFullMask = 0xffffffffu;

__device__ float warp_reduce_max(float value) {
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 16));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 8));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 4));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 2));
    value = fmaxf(value, __shfl_down_sync(kFullMask, value, 1));
    return value;
}

__device__ float warp_reduce_sum(float value) {
    value += __shfl_down_sync(kFullMask, value, 16);
    value += __shfl_down_sync(kFullMask, value, 8);
    value += __shfl_down_sync(kFullMask, value, 4);
    value += __shfl_down_sync(kFullMask, value, 2);
    value += __shfl_down_sync(kFullMask, value, 1);
    return value;
}

__device__ float block_reduce_max(float value) {
    __shared__ float warp_values[kBlockSize / 32];
    int lane = threadIdx.x & 31;
    int warp_id = threadIdx.x >> 5;

    value = warp_reduce_max(value);
    if (lane == 0) {
        warp_values[warp_id] = value;
    }
    __syncthreads();

    // 第二轮只让第一个 warp 归约各 warp 的局部结果。
    value = (threadIdx.x < kBlockSize / 32) ? warp_values[lane] : -FLT_MAX;
    if (warp_id == 0) {
        value = warp_reduce_max(value);
    }
    return value;
}

__device__ float block_reduce_sum(float value) {
    __shared__ float warp_values[kBlockSize / 32];
    int lane = threadIdx.x & 31;
    int warp_id = threadIdx.x >> 5;

    value = warp_reduce_sum(value);
    if (lane == 0) {
        warp_values[warp_id] = value;
    }
    __syncthreads();

    // 非首个 warp 的线程提供 0，避免污染最终 block sum。
    value = (threadIdx.x < kBlockSize / 32) ? warp_values[lane] : 0.0f;
    if (warp_id == 0) {
        value = warp_reduce_sum(value);
    }
    return value;
}

__global__ void softmax_basic_kernel(const float *__restrict__ input,
                                     float *__restrict__ output,
                                     int batch_size,
                                     int seq_len) {
    int row = blockIdx.x;
    if (row >= batch_size) {
        return;
    }

    int tid = threadIdx.x;
    const float *row_input = input + row * seq_len;
    float *row_output = output + row * seq_len;

    // 先减去行最大值，避免 expf 在大输入上溢出。
    float local_max = -FLT_MAX;
    for (int i = tid; i < seq_len; i += blockDim.x) {
        local_max = fmaxf(local_max, row_input[i]);
    }
    float max_value = block_reduce_max(local_max);

    __shared__ float shared_max;
    if (tid == 0) {
        shared_max = max_value;
    }
    __syncthreads();

    float local_sum = 0.0f;
    for (int i = tid; i < seq_len; i += blockDim.x) {
        local_sum += expf(row_input[i] - shared_max);
    }
    float sum_value = block_reduce_sum(local_sum);

    __shared__ float shared_sum;
    if (tid == 0) {
        shared_sum = sum_value;
    }
    __syncthreads();

    for (int i = tid; i < seq_len; i += blockDim.x) {
        row_output[i] = expf(row_input[i] - shared_max) / shared_sum;
    }
}

void launch_softmax_basic(const float *input, float *output, int batch_size, int seq_len) {
    if (batch_size <= 0 || seq_len <= 0) {
        return;
    }

    softmax_basic_kernel<<<batch_size, kBlockSize>>>(input, output, batch_size, seq_len);
}
