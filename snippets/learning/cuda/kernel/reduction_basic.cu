#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void reduction_basic_kernel(const float *input, float *output, int N) {
    __shared__ float shared_sum[kBlockSize];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + tid;
    shared_sum[tid] = (idx < N) ? input[idx] : 0.0f;
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

void launch_reduction_basic(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }
    int blocks = (N + kBlockSize - 1) / kBlockSize;
    reduction_basic_kernel<<<blocks, kBlockSize>>>(input, output, N);
}
