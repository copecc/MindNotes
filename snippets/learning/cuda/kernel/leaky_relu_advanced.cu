#include <cuda_runtime.h>

__global__ void leaky_relu_advanced_kernel(const float *input, float *output, float alpha, int N) {
    // 网格跨步访问 (Grid-Stride Loop)，能让固定尺寸的Grid处理任意大数据
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = idx; i < N; i += stride) {
        float x = input[i];
        output[i] = x >= 0.0f ? x : alpha * x;
    }
}

void launch_leaky_relu_advanced(const float *input, float *output, float alpha, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    leaky_relu_advanced_kernel<<<blocks, threads>>>(input, output, alpha, N);
}
