#include <cuda_runtime.h>

constexpr int kBlockSize = 256;
constexpr unsigned kFullMask = 0xffffffffu;

__device__ float warp_reduce_sum(float value) {
    value += __shfl_down_sync(kFullMask, value, 16);
    value += __shfl_down_sync(kFullMask, value, 8);
    value += __shfl_down_sync(kFullMask, value, 4);
    value += __shfl_down_sync(kFullMask, value, 2);
    value += __shfl_down_sync(kFullMask, value, 1);
    return value;
}

__global__ void reduction_advanced_kernel(const float *input, float *output, int N) {
    int tid = threadIdx.x;
    int lane = tid & 31;
    int warp_id = tid >> 5;

    float thread_sum = 0.0f;
    // 每个线程先处理两个相隔 blockDim.x 的元素，减少后续归约阶段的输入规模。
    int idx = blockIdx.x * (blockDim.x * 2) + tid;
    int stride = gridDim.x * (blockDim.x * 2);
    for (int i = idx; i < N; i += stride) {
        thread_sum += input[i];
        if (i + blockDim.x < N) {
            thread_sum += input[i + blockDim.x];
        }
    }

    thread_sum = warp_reduce_sum(thread_sum);

    // 只把每个 warp 的 lane 0 结果写入 shared memory，减少同步和 shared memory 访问次数。
    __shared__ float warp_sums[kBlockSize / 32];
    if (lane == 0) {
        warp_sums[warp_id] = thread_sum;
    }
    __syncthreads();

    if (warp_id == 0) {
        float block_sum = (tid < kBlockSize / 32) ? warp_sums[lane] : 0.0f;
        block_sum = warp_reduce_sum(block_sum);
        if (lane == 0) {
            atomicAdd(output, block_sum);
        }
    }
}

void launch_reduction_advanced(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }
    int blocks = (N + kBlockSize * 2 - 1) / (kBlockSize * 2);
    reduction_advanced_kernel<<<blocks, kBlockSize>>>(input, output, N);
}
