#include <cuda_runtime.h>

constexpr int kTileDim = 32;
constexpr int kBlockRows = 8;

__global__ void matrix_transpose_advanced_kernel(const float *__restrict__ input,
                                                 float *__restrict__ output,
                                                 int width,
                                                 int height) {
    // Shared Memory Tile + 1 进行 padding 预防 bank conflict
    __shared__ float tile[kTileDim][kTileDim + 1];

    int x = blockIdx.x * kTileDim + threadIdx.x;
    int y = blockIdx.y * kTileDim + threadIdx.y;

    // 分块（tile）方式加载数据到 shared memory
    // 读取 input 时按行合并访问（coalesced access）
    for (int i = 0; i < kTileDim; i += kBlockRows) {
        int current_y = y + i;
        if (x < width && current_y < height) {
            tile[threadIdx.y + i][threadIdx.x] = input[current_y * width + x];
        }
    }

    __syncthreads();

    // 计算转置后的写回目标坐标
    x = blockIdx.y * kTileDim + threadIdx.x;
    y = blockIdx.x * kTileDim + threadIdx.y;
    
    // 写回 output 时相邻 threadIdx.x 对应转置矩阵同一行的连续列。
    for (int i = 0; i < kTileDim; i += kBlockRows) {
        int current_y = y + i;
        if (x < height && current_y < width) {
            output[x * width + current_y] = tile[threadIdx.x][threadIdx.y + i];
        }
    }
}

void launch_matrix_transpose_advanced(const float *input, float *output, int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    dim3 threads(kTileDim, kBlockRows);
    dim3 blocks((cols + kTileDim - 1) / kTileDim,
                (rows + kTileDim - 1) / kTileDim);
    matrix_transpose_advanced_kernel<<<blocks, threads>>>(input, output, cols, rows);
}
