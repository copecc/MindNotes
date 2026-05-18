#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void exclusive_scan_blocks_kernel(const float *input, float *output, float *block_sums, int N) {
    __shared__ float shared[kBlockSize];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + tid;
    shared[tid] = (idx < N) ? input[idx] : 0.0f;
    __syncthreads();

    for (int stride = 1; stride < blockDim.x; stride <<= 1) {
        int index = ((tid + 1) * stride * 2) - 1;
        if (index < blockDim.x) {
            shared[index] += shared[index - stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        // 这里先产出 block_sums；完整多 block scan 还需要对它再做一次 scan。
        block_sums[blockIdx.x] = shared[blockDim.x - 1];
        shared[blockDim.x - 1] = 0.0f;
    }
    __syncthreads();

    for (int stride = blockDim.x >> 1; stride > 0; stride >>= 1) {
        int index = ((tid + 1) * stride * 2) - 1;
        if (index < blockDim.x) {
            float left = shared[index - stride];
            shared[index - stride] = shared[index];
            shared[index] += left;
        }
        __syncthreads();
    }

    if (idx < N) {
        output[idx] = shared[tid];
    }
}

__global__ void add_block_offsets_kernel(float *output, const float *block_offsets, int N) {
    // block_offsets 是每个 block 左侧所有 block_sums 的 exclusive scan。
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[idx] += block_offsets[blockIdx.x];
    }
}

void launch_exclusive_scan_blocks(const float *input, float *output, float *block_sums, int N) {
    if (N <= 0) {
        return;
    }
    int blocks = (N + kBlockSize - 1) / kBlockSize;
    exclusive_scan_blocks_kernel<<<blocks, kBlockSize>>>(input, output, block_sums, N);
}

void launch_add_block_offsets(float *output, const float *block_offsets, int N) {
    if (N <= 0) {
        return;
    }
    int blocks = (N + kBlockSize - 1) / kBlockSize;
    add_block_offsets_kernel<<<blocks, kBlockSize>>>(output, block_offsets, N);
}
