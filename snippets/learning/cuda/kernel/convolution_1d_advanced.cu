#include <cuda_runtime.h>

constexpr int kBlockSize = 256;
constexpr int kThreadOutputs = 4;
constexpr int kMaxKernelSize = 4096;
__constant__ float c_kernel[kMaxKernelSize];

__global__ void convolution_1d_advanced_kernel(const float *input,
                                               float *output,
                                               int input_size,
                                               int kernel_size) {
    int output_size = input_size - kernel_size + 1;
    int outputs_per_block = blockDim.x * kThreadOutputs;
    int block_output_start = blockIdx.x * outputs_per_block;

    extern __shared__ float shared_input[];
    // 相邻输出共享大部分输入窗口，先把整块输入窗口搬入 shared memory 减少重复 global load。
    int tile_input_size = min(outputs_per_block + kernel_size - 1, input_size - block_output_start);
    for (int t = threadIdx.x; t < tile_input_size; t += blockDim.x) {
        shared_input[t] = input[block_output_start + t];
    }
    __syncthreads();

    float acc[kThreadOutputs] = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int j = 0; j < kernel_size; ++j) {
        float k = c_kernel[j];
#pragma unroll
        for (int o = 0; o < kThreadOutputs; ++o) {
            // 让同一轮 o 内的连续线程写连续输出，避免 thread coarsening 破坏合并访存。
            int out_idx = block_output_start + o * blockDim.x + threadIdx.x;
            if (out_idx < output_size) {
                acc[o] += shared_input[o * blockDim.x + threadIdx.x + j] * k;
            }
        }
    }

#pragma unroll
    for (int o = 0; o < kThreadOutputs; ++o) {
        int out_idx = block_output_start + o * blockDim.x + threadIdx.x;
        if (out_idx < output_size) {
            output[out_idx] = acc[o];
        }
    }
}

void launch_convolution_1d_advanced(const float *input,
                                    const float *kernel,
                                    float *output,
                                    int input_size,
                                    int kernel_size) {
    if (kernel_size <= 0 || kernel_size > input_size || kernel_size > kMaxKernelSize) {
        return;
    }
    int output_size = input_size - kernel_size + 1;
    int outputs_per_block = kBlockSize * kThreadOutputs;
    int blocks = (output_size + outputs_per_block - 1) / outputs_per_block;
    cudaMemcpyToSymbol(c_kernel, kernel, kernel_size * sizeof(float), 0, cudaMemcpyHostToDevice);
    // shared tile 覆盖本 block 的输出窗口以及卷积右侧 halo。
    size_t shared_bytes = static_cast<size_t>(outputs_per_block + kernel_size - 1) * sizeof(float);
    convolution_1d_advanced_kernel<<<blocks, kBlockSize, shared_bytes>>>(input, output, input_size, kernel_size);
}
