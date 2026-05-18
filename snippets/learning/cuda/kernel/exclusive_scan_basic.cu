#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void exclusive_scan_basic_kernel(const float *input, float *output, int N) {
    __shared__ float shared[kBlockSize];

    int tid = threadIdx.x;
    shared[tid] = (tid < N) ? input[tid] : 0.0f;
    __syncthreads();

    // Upsweep 建树，先把区间和收拢到右端点。
    for (int stride = 1; stride < blockDim.x; stride <<= 1) {
        int index = ((tid + 1) * stride * 2) - 1;
        if (index < blockDim.x) {
            shared[index] += shared[index - stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        shared[blockDim.x - 1] = 0.0f;
    }
    __syncthreads();

    // Downsweep 把父节点的前缀值向左下传递。
    for (int stride = blockDim.x >> 1; stride > 0; stride >>= 1) {
        int index = ((tid + 1) * stride * 2) - 1;
        if (index < blockDim.x) {
            float left = shared[index - stride];
            shared[index - stride] = shared[index];
            shared[index] += left;
        }
        __syncthreads();
    }

    if (tid < N) {
        output[tid] = shared[tid];
    }
}

void launch_exclusive_scan_basic(const float *input, float *output, int N) {
    if (N <= 0 || N > kBlockSize) {
        return;
    }
    exclusive_scan_basic_kernel<<<1, kBlockSize>>>(input, output, N);
}
