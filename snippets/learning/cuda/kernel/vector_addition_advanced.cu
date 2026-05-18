#include <cuda_runtime.h>
#include <cstdint>

__global__ void vector_addition_advanced_kernel(const float4 *a, const float4 *b, float4 *output, int N4) {
    // 处理矢量化数据，一次读取4个float
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N4) {
        float4 av = a[idx];
        float4 bv = b[idx];
        output[idx] = make_float4(av.x + bv.x, av.y + bv.y, av.z + bv.z, av.w + bv.w);
    }
}

__global__ void vector_addition_tail_kernel(const float *a, const float *b, float *output, int start, int N) {
    int idx = start + blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[idx] = a[idx] + b[idx];
    }
}

__global__ void vector_addition_scalar_kernel(const float *a, const float *b, float *output, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        output[idx] = a[idx] + b[idx];
    }
}

void launch_vector_addition_advanced(const float *a, const float *b, float *output, int N) {
    if (N <= 0) {
        return;
    }

    bool aligned = (reinterpret_cast<std::uintptr_t>(a) % alignof(float4) == 0) &&
                   (reinterpret_cast<std::uintptr_t>(b) % alignof(float4) == 0) &&
                   (reinterpret_cast<std::uintptr_t>(output) % alignof(float4) == 0);
    if (!aligned) {
        int threads = 256;
        int blocks = (N + threads - 1) / threads;
        vector_addition_scalar_kernel<<<blocks, threads>>>(a, b, output, N);
        return;
    }

    int N4 = N / 4;
    int tail = N % 4;
    int threads = 256;

    if (N4 > 0) {
        int blocks = (N4 + threads - 1) / threads;
        vector_addition_advanced_kernel<<<blocks, threads>>>(reinterpret_cast<const float4 *>(a),
                                                             reinterpret_cast<const float4 *>(b),
                                                             reinterpret_cast<float4 *>(output),
                                                             N4);
    }
    if (tail > 0) {
        vector_addition_tail_kernel<<<1, 32>>>(a, b, output, N4 * 4, N);
    }
}
