#include <cuda_runtime.h>

__global__ void count_array_element2d_basic_kernel(const int *input, int *count, int rows, int cols, int target) {
    // 恢复二维线程的行坐标与列坐标
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    
    // 检查越界，如果命中目标值，使用原子加累加全局计数器
    if (row < rows && col < cols && input[row * cols + col] == target) {
        atomicAdd(count, 1);
    }
}

void launch_count_array_element2d_basic(const int *input, int *count, int rows, int cols, int target) {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((cols + threads.x - 1) / threads.x,
                (rows + threads.y - 1) / threads.y);
    count_array_element2d_basic_kernel<<<blocks, threads>>>(input, count, rows, cols, target);
}
