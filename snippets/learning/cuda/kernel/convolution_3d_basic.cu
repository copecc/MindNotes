#include <cuda_runtime.h>

__global__ void convolution_3d_basic_kernel(const float *input,
                                            float *output,
                                            const float *filter,
                                            int width,
                                            int height,
                                            int depth,
                                            int filter_size) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;

    if (x < width && y < height && z < depth) {
        float sum = 0.0f;
        int filter_half = filter_size / 2;
        int filter_area = filter_size * filter_size;

        for (int d = -filter_half; d <= filter_half; ++d) {
            for (int r = -filter_half; r <= filter_half; ++r) {
                for (int c = -filter_half; c <= filter_half; ++c) {
                    int cur_z = z + d;
                    int cur_y = y + r;
                    int cur_x = x + c;

                    if (cur_z >= 0 && cur_z < depth && cur_y >= 0 && cur_y < height && cur_x >= 0 && cur_x < width) {
                        int input_idx = cur_z * width * height + cur_y * width + cur_x;
                        int filter_z = d + filter_half;
                        int filter_y = r + filter_half;
                        int filter_x = c + filter_half;
                        int filter_idx = filter_z * filter_area + filter_y * filter_size + filter_x;
                        sum += input[input_idx] * filter[filter_idx];
                    }
                }
            }
        }

        output[z * width * height + y * width + x] = sum;
    }
}

void launch_convolution_3d_basic(const float *input,
                                 float *output,
                                 const float *filter,
                                 int width,
                                 int height,
                                 int depth,
                                 int filter_size) {
    if (width <= 0 || height <= 0 || depth <= 0 || filter_size <= 0 || filter_size % 2 == 0) {
        return;
    }

    dim3 threads(8, 8, 4);
    dim3 blocks((width + threads.x - 1) / threads.x,
                (height + threads.y - 1) / threads.y,
                (depth + threads.z - 1) / threads.z);
    convolution_3d_basic_kernel<<<blocks, threads>>>(input, output, filter, width, height, depth, filter_size);
}
