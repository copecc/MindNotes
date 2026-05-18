#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void dot_product_basic_kernel(const float *a, const float *b, float *output, int N) {
    __shared__ float shared_sum[kBlockSize];

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;

    shared_sum[tid] = (idx < N) ? a[idx] * b[idx] : 0.0f;
    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            shared_sum[tid] += shared_sum[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        atomicAdd(output, shared_sum[0]);
    }
}

void launch_dot_product_basic(const float *a, const float *b, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int blocks = (N + kBlockSize - 1) / kBlockSize;
    dot_product_basic_kernel<<<blocks, kBlockSize>>>(a, b, output, N);
}
