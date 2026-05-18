#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void mean_squared_error_basic_kernel(const float *preds, const float *labels, float *mse, int N) {
    __shared__ float shared_sum[kBlockSize];

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;

    float local_err = 0.0f;
    if (idx < N) {
        float diff = preds[idx] - labels[idx];
        local_err = diff * diff;
    }
    shared_sum[tid] = local_err;
    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            shared_sum[tid] += shared_sum[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        atomicAdd(mse, shared_sum[0]);
    }
}

void launch_mean_squared_error_basic(const float *preds, const float *labels, float *mse, int N) {
    if (N <= 0) {
        return;
    }

    int blocks = (N + kBlockSize - 1) / kBlockSize;
    mean_squared_error_basic_kernel<<<blocks, kBlockSize>>>(preds, labels, mse, N);
}
