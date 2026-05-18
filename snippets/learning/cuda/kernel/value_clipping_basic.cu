#include <cuda_runtime.h>

__global__ void value_clipping_basic_kernel(const float *input,
                                            float *output,
                                            float min_val,
                                            float max_val,
                                            int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        float value = input[idx];
        value = value < min_val ? min_val : value;
        value = value > max_val ? max_val : value;
        output[idx] = value;
    }
}

void launch_value_clipping_basic(const float *input,
                                 float *output,
                                 float min_val,
                                 float max_val,
                                 int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    int blocks = (N + threads - 1) / threads;
    value_clipping_basic_kernel<<<blocks, threads>>>(input, output, min_val, max_val, N);
}
