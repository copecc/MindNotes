#include <cuda_runtime.h>

__global__ void rgb_to_grayscale_basic_kernel(const unsigned char *rgb,
                                              unsigned char *gray,
                                              int width,
                                              int height) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (col < width && row < height) {
        int gray_idx = row * width + col;
        int rgb_idx = gray_idx * 3;
        unsigned char r = rgb[rgb_idx];
        unsigned char g = rgb[rgb_idx + 1];
        unsigned char b = rgb[rgb_idx + 2];
        gray[gray_idx] = static_cast<unsigned char>(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

void launch_rgb_to_grayscale_basic(const unsigned char *rgb,
                                   unsigned char *gray,
                                   int width,
                                   int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((width + threads.x - 1) / threads.x,
                (height + threads.y - 1) / threads.y);
    rgb_to_grayscale_basic_kernel<<<blocks, threads>>>(rgb, gray, width, height);
}
