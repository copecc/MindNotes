#include <cuda_runtime.h>

__global__ void sigmoid_advanced_kernel(const float *input, float *output, int N) {
    // 网格跨步访问 (Grid-Stride Loop)，能让固定尺寸的Grid处理任意大数据
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = idx; i < N; i += stride) {
        output[i] = 1.0f / (1.0f + __expf(-input[i]));
    }
}

void launch_sigmoid_advanced(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    sigmoid_advanced_kernel<<<blocks, threads>>>(input, output, N);
}
