#include <cuda_runtime.h>

constexpr int kBlockSize = 256;

__global__ void segmented_prefix_sum_basic_kernel(const float *input,
                                                  const int *flags,
                                                  float *output,
                                                  int N) {
    __shared__ float shared_data[kBlockSize];
    __shared__ int shared_head[kBlockSize];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + tid;

    shared_data[tid] = (idx < N) ? input[idx] : 0.0f;
    shared_head[tid] = (idx < N && flags[idx] != 0) ? tid : -1;
    __syncthreads();

    // shared_head 记录当前前缀能跨过的最近段起点，遇到新段标记后停止吸收左侧累加值。
    for (int stride = 1; stride < blockDim.x; stride <<= 1) {
        float left_data = 0.0f;
        int left_head = -1;
        if (tid >= stride) {
            left_data = shared_data[tid - stride];
            left_head = shared_head[tid - stride];
        }
        __syncthreads();

        if (tid >= stride && left_head >= shared_head[tid]) {
            shared_data[tid] += left_data;
            shared_head[tid] = left_head;
        }
        __syncthreads();
    }

    if (idx < N) {
        output[idx] = shared_data[tid];
    }
}

void launch_segmented_prefix_sum_basic(const float *input, const int *flags, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int blocks = (N + kBlockSize - 1) / kBlockSize;
    segmented_prefix_sum_basic_kernel<<<blocks, kBlockSize>>>(input, flags, output, N);
}
