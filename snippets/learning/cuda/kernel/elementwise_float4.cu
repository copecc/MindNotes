#include <cuda_runtime.h>
#include <cstdint>

__global__ void elementwise_float4_kernel(const float4 *__restrict__ input4,
                                          float4 *__restrict__ output4,
                                          int packed_count) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < packed_count) {
        output4[idx] = input4[idx];
    }
}

__global__ void elementwise_scalar_kernel(const float *__restrict__ input,
                                          float *__restrict__ output,
                                          int start,
                                          int N) {
    int idx = start + blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[idx] = input[idx];
    }
}

void launch_elementwise_float4(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    constexpr int kThreads = 256;
    bool aligned = (reinterpret_cast<std::uintptr_t>(input) % alignof(float4) == 0) &&
                   (reinterpret_cast<std::uintptr_t>(output) % alignof(float4) == 0);
    // 非 16 字节对齐时退回标量路径，避免未对齐 float4 访问。
    if (!aligned) {
        int blocks = (N + kThreads - 1) / kThreads;
        elementwise_scalar_kernel<<<blocks, kThreads>>>(input, output, 0, N);
        return;
    }

    int packed_count = N / 4;
    int tail_start = packed_count * 4;

    if (packed_count > 0) {
        int blocks = (packed_count + kThreads - 1) / kThreads;
        auto *input4 = reinterpret_cast<const float4 *>(input);
        auto *output4 = reinterpret_cast<float4 *>(output);
        elementwise_float4_kernel<<<blocks, kThreads>>>(input4, output4, packed_count);
    }

    if (tail_start < N) {
        elementwise_scalar_kernel<<<1, 32>>>(input, output, tail_start, N);
    }
}
