#include <cuda_runtime.h>

constexpr int kBlockSize = 256;
constexpr int kThreadOutputs = 4;
constexpr int kMaxKernelSize = 4096;
__constant__ float c_kernel[kMaxKernelSize];

__global__ void convolution_1d_basic_kernel(const float *input,
                                           const float *kernel,
                                           float *output,
                                           int input_size,
                                           int kernel_size) {
    int output_idx = blockIdx.x * blockDim.x + threadIdx.x;
    int output_size = input_size - kernel_size + 1;
    if (output_idx < output_size) {
        float sum = 0.0f;
        for (int j = 0; j < kernel_size; ++j) {
            sum += input[output_idx + j] * kernel[j];
        }
        output[output_idx] = sum;
    }
}

void launch_convolution_1d_basic(const float *input,
                                  const float *kernel,
                                  float *output,
                                  int input_size,
                                  int kernel_size) {
    if (kernel_size <= 0 || kernel_size > input_size) {
        return;
    }
    int output_size = input_size - kernel_size + 1;
    int blocks = (output_size + kBlockSize - 1) / kBlockSize;
    convolution_1d_basic_kernel<<<blocks, kBlockSize>>>(input, kernel, output, input_size, kernel_size);
}
