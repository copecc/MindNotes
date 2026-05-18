#include <cuda_runtime.h>

__global__ void matrix_transpose_basic_kernel(const float *input, float *output, int rows, int cols) {
    // 恢复当前的行列坐标
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    
    // 检查越界
    if (row < rows && col < cols) {
        // 直接从输入矩阵读 (row, col) 的数据，写入输出矩阵的 (col, row) 的位置
        // 注意：这会导致对 output 的写入不能享受合并访存 (coalesced memory access)
        output[col * rows + row] = input[row * cols + col];
    }
}

void launch_matrix_transpose_basic(const float *input, float *output, int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    dim3 threads(16, 16);
    dim3 blocks((cols + threads.x - 1) / threads.x,
                (rows + threads.y - 1) / threads.y);
    matrix_transpose_basic_kernel<<<blocks, threads>>>(input, output, rows, cols);
}
