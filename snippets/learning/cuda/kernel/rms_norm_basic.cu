#include <cuda_runtime.h>

// 针对单行的 RMSNorm (Root Mean Square Normalization)
// 通常分配 1 个 Block 负责 1 个样本 (1 行)，BlockDim.x 对应 hidden_size
__global__ void rms_norm_basic_kernel(const float* input, float* output, const float* weight, float epsilon, int N) {
    __shared__ float s_variance;

    // 每个线程先累积一个元素的平方，后续再做 block 归约。
    __shared__ float s_sum[1024]; // 假设最多1024线程

    int tid = threadIdx.x;

    float local_val = (tid < N) ? input[tid] : 0.0f;
    s_sum[tid] = local_val * local_val;
    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            s_sum[tid] += s_sum[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        float mean_square = s_sum[0] / (float)N;
        s_variance = rsqrtf(mean_square + epsilon);
    }
    __syncthreads();

    if (tid < N) {
        output[tid] = input[tid] * s_variance * weight[tid];
    }
}
