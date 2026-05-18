#include <cuda_runtime.h>

__global__ void convolution_2d_basic_kernel(const float *input,
                                            float *output,
                                            const float *filter,
                                            int width,
                                            int height,
                                            int filter_size) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (col < width && row < height) {
        float sum = 0.0f;
        int filter_half = filter_size / 2;

        for (int r = -filter_half; r <= filter_half; ++r) {
            for (int c = -filter_half; c <= filter_half; ++c) {
                int cur_row = row + r;
                int cur_col = col + c;

                float value = 0.0f;
                if (cur_row >= 0 && cur_row < height && cur_col >= 0 && cur_col < width) {
                    value = input[cur_row * width + cur_col];
                }

                int filter_r = r + filter_half;
                int filter_c = c + filter_half;
                sum += value * filter[filter_r * filter_size + filter_c];
            }
        }

        output[row * width + col] = sum;
    }
}

void launch_convolution_2d_basic(const float *input,
                                 float *output,
                                 const float *filter,
                                 int width,
                                 int height,
                                 int filter_size) {
    if (width <= 0 || height <= 0 || filter_size <= 0 || filter_size % 2 == 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((width + threads.x - 1) / threads.x,
                (height + threads.y - 1) / threads.y);
    convolution_2d_basic_kernel<<<blocks, threads>>>(input, output, filter, width, height, filter_size);
}
