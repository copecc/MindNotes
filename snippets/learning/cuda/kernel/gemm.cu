#include <cuda_runtime.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "assert.h"

namespace {

constexpr std::array<int, 4> kSizes = {128, 256, 512, 1024};
constexpr int kWarmupRuns           = 3;
constexpr int kTimedRuns            = 5;

struct RunResult {
  std::array<double, kSizes.size()> times{};
  std::array<double, kSizes.size()> gflops{};
  std::array<double, kSizes.size()> bandwidth{};
};

using GemmFn          = void (*)(float *, float *, float *, int, int, int);
using MetricValues    = std::array<double, kSizes.size()>;
using RunResultMetric = MetricValues RunResult::*;

struct GemmVariant {
  const char *name;
  GemmFn fn;
};

using VariantList = std::array<GemmVariant, 6>;
using ResultList  = std::array<RunResult, VariantList{}.size()>;

struct Gemm6Config {
  const char *name;
  int coarse_x;
  int coarse_y;
  int tiles_Arows;
  int tiles_Acols;
  int tiles_Bcols;
};

constexpr std::array<Gemm6Config, 5> kGemm6Configs = {
    {
     {"G6 128x128 BK16 8x8", 8, 8, 128, 16, 128},
     {"G6 128x64 BK16 8x8", 8, 8, 128, 16, 64},
     {"G6 128x128 BK16 8x4", 8, 4, 128, 16, 128},
     {"G6 64x128 BK16 4x8", 4, 8, 64, 16, 128},
     {"G6 64x64 BK16 4x4", 4, 4, 64, 16, 64},
     }
};

using Gemm6SweepResults = std::array<RunResult, kGemm6Configs.size()>;

}  // namespace

// GEMM 01 -- NAIVE IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
__global__ void naive_kernel(float *A, float *B, float *C, int M, int N, int K) {
  // 2D thread layout: one thread computes one output element C[row, col].
  const int row = blockDim.x * blockIdx.x + threadIdx.x;
  const int col = blockDim.y * blockIdx.y + threadIdx.y;

  if (row < M && col < N) {
    float dot_prod = 0;
    // Read A[row, 0:K] and B[0:K, col] directly from global memory.
    for (int k = 0; k < K; ++k) { dot_prod += A[row * K + k] * B[k * N + col]; }
    // Write one output element: C[row, col].
    C[row * N + col] = dot_prod;
  }
}

void gemm1(float *A, float *B, float *C, int M, int N, int K) {
  constexpr int BLOCK_X = 32, BLOCK_Y = 32;
  dim3 dim_block(BLOCK_X, BLOCK_Y, 1);
  dim3 dim_grid((M + BLOCK_X - 1) / BLOCK_X, (N + BLOCK_Y - 1) / BLOCK_Y, 1);
  naive_kernel<<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}

// GEMM 02 -- COALESCED MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
__global__ void coalesced_kernel(float *A, float *B, float *C, int M, int N, int K) {
  // Swap the mapping so warp lanes advance along columns for more coalesced global reads from B and
  // writes to C[row, col].
  const int col = blockDim.x * blockIdx.x + threadIdx.x;
  const int row = blockDim.y * blockIdx.y + threadIdx.y;

  if (row < M && col < N) {
    float dot_prod = 0;
    // Still reads A[row, 0:K] and B[0:K, col] from global memory, but with a more
    // cache/transaction-friendly thread layout.
    for (int k = 0; k < K; ++k) { dot_prod += A[row * K + k] * B[k * N + col]; }
    C[row * N + col] = dot_prod;
  }
}

void gemm2(float *A, float *B, float *C, int M, int N, int K) {
  constexpr int BLOCK_X = 32, BLOCK_Y = 32;
  dim3 dim_block(BLOCK_X, BLOCK_Y, 1);
  dim3 dim_grid((M + BLOCK_X - 1) / BLOCK_X, (N + BLOCK_Y - 1) / BLOCK_Y, 1);
  coalesced_kernel<<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}

// GEMM 03 -- TILED MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
template <int TILE_WIDTH>
__global__ void tiled_kernel(float *A, float *B, float *C, int M, int N, int K) {
  assert(TILE_WIDTH == blockDim.x);
  assert(TILE_WIDTH == blockDim.y);

  const int block_x  = blockIdx.x;
  const int block_y  = blockIdx.y;
  const int thread_x = threadIdx.x;
  const int thread_y = threadIdx.y;

  // One thread still owns one output element C[row, col], but the block reuses a TILE_WIDTH x
  // TILE_WIDTH patch.
  const int row       = TILE_WIDTH * block_y + thread_y;
  const int col       = TILE_WIDTH * block_x + thread_x;
  const int num_tiles = (K + TILE_WIDTH - 1) / TILE_WIDTH;

  // sh_A holds A[block_y*TILE_WIDTH:(block_y+1)*TILE_WIDTH, tile*TILE_WIDTH:(tile+1)*TILE_WIDTH],
  // sh_B holds B[tile*TILE_WIDTH:(tile+1)*TILE_WIDTH, block_x*TILE_WIDTH:(block_x+1)*TILE_WIDTH].
  __shared__ float sh_A[TILE_WIDTH][TILE_WIDTH];
  __shared__ float sh_B[TILE_WIDTH][TILE_WIDTH];

  float dot_prod = 0;
  for (int tile = 0; tile < num_tiles; ++tile) {
    // Each thread loads one element of A and one element of B into shared memory.
    // Boundary checks handle edge cases when M/N/K is not divisible by TILE_WIDTH.
    if (row < M && tile * TILE_WIDTH + thread_x < K) {
      sh_A[thread_y][thread_x] = A[row * K + tile * TILE_WIDTH + thread_x];
    } else {
      sh_A[thread_y][thread_x] = 0.0f;
    }

    if (tile * TILE_WIDTH + thread_y < K && col < N) {
      sh_B[thread_y][thread_x] = B[(tile * TILE_WIDTH + thread_y) * N + col];
    } else {
      sh_B[thread_y][thread_x] = 0.0f;
    }

    __syncthreads();

    // Shared memory turns K-step global loads into tile reuse inside the block.
    for (int k_tile = 0; k_tile < TILE_WIDTH; ++k_tile) {
      dot_prod += sh_A[thread_y][k_tile] * sh_B[k_tile][thread_x];
    }
    __syncthreads();
  }

  if (row < M && col < N) { C[row * N + col] = dot_prod; }
}

void gemm3(float *A, float *B, float *C, int M, int N, int K) {
  // Optimized/original parameters: TILE_WIDTH=32.
  constexpr int TILE_WIDTH = 32;

  dim3 dim_block(TILE_WIDTH, TILE_WIDTH, 1);
  dim3 dim_grid((M + TILE_WIDTH - 1) / TILE_WIDTH, (N + TILE_WIDTH - 1) / TILE_WIDTH, 1);
  tiled_kernel<TILE_WIDTH><<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}

// GEMM 04 -- COARSE 1D MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
template <int COARSE_FACTOR, int TILE_A_ROWS, int TILE_A_COLS, int TILE_B_COLS>
__global__ void coarse1D_kernel(float *A, float *B, float *C, int M, int N, int K) {
  const int block_x  = blockIdx.x;
  const int block_y  = blockIdx.y;
  const int thread_x = threadIdx.x;

  // Threads are laid out as a 1D block; each thread loads one A element and one B element into
  // shared memory.
  const int A_view_y = thread_x / TILE_A_COLS;
  const int A_view_x = thread_x % TILE_A_COLS;
  const int B_view_y = thread_x / TILE_B_COLS;
  const int B_view_x = thread_x % TILE_B_COLS;

  // Each thread computes COARSE_FACTOR outputs stacked along rows for the same output column
  // C[row:row+COARSE_FACTOR, col].
  const int row       = TILE_A_ROWS * block_y + COARSE_FACTOR * (thread_x / TILE_B_COLS);
  const int col       = TILE_B_COLS * block_x + (thread_x % TILE_B_COLS);
  const int num_tiles = (K + TILE_A_COLS - 1) / TILE_A_COLS;

  // sh_A caches A[block_y*TILE_A_ROWS:(block_y+1)*TILE_A_ROWS,
  // tile*TILE_A_COLS:(tile+1)*TILE_A_COLS], sh_B caches B[tile*TILE_A_COLS:(tile+1)*TILE_A_COLS,
  // block_x*TILE_B_COLS:(block_x+1)*TILE_B_COLS].
  __shared__ float sh_A[TILE_A_ROWS][TILE_A_COLS];
  __shared__ float sh_B[TILE_A_COLS][TILE_B_COLS];

  float dot_prod[COARSE_FACTOR] = {0.0f};
  for (int tile = 0; tile < num_tiles; ++tile) {
    if (block_y * TILE_A_ROWS + A_view_y < M && tile * TILE_A_COLS + A_view_x < K) {
      sh_A[A_view_y][A_view_x]
          = A[(block_y * TILE_A_ROWS + A_view_y) * K + tile * TILE_A_COLS + A_view_x];
    } else {
      sh_A[A_view_y][A_view_x] = 0.0f;
    }

    if (tile * TILE_A_COLS + B_view_y < K && block_x * TILE_B_COLS + B_view_x < N) {
      sh_B[B_view_y][B_view_x]
          = B[(tile * TILE_A_COLS + B_view_y) * N + block_x * TILE_B_COLS + B_view_x];
    } else {
      sh_B[B_view_y][B_view_x] = 0.0f;
    }
    __syncthreads();

    // Register blocking: reuse one B value across COARSE_FACTOR rows before moving to the next K
    // step.
    for (int k_tile = 0; k_tile < TILE_A_COLS; ++k_tile) {
      const float B_val = sh_B[k_tile][B_view_x];
      for (int c = 0; c < COARSE_FACTOR; ++c) {
        dot_prod[c] += sh_A[B_view_y * COARSE_FACTOR + c][k_tile] * B_val;
      }
    }
    __syncthreads();
  }

  for (int c = 0; c < COARSE_FACTOR; ++c) {
    if (row + c < M && col < N) { C[(row + c) * N + col] = dot_prod[c]; }
  }
}

void gemm4(float *A, float *B, float *C, int M, int N, int K) {
  // Optimized/original parameters: COARSE_FACTOR=8, TILE_A_ROWS=64, TILE_A_COLS=8, TILE_B_COLS=64.
  constexpr int COARSE_FACTOR = 8;
  constexpr int TILE_A_ROWS   = 64;
  constexpr int TILE_A_COLS   = 8;
  constexpr int TILE_B_COLS   = 64;

  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(TILE_A_ROWS * TILE_B_COLS / COARSE_FACTOR);
  coarse1D_kernel<COARSE_FACTOR, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}

// GEMM 05 -- COARSE 2D MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
template <int COARSE_X, int COARSE_Y, int TILE_A_ROWS, int TILE_A_COLS, int TILE_B_COLS>
__global__ void coarse2D_kernel(float *A, float *B, float *C, int M, int N, int K) {
  constexpr int num_threads = TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y);
  static_assert(num_threads % TILE_A_COLS == 0);
  static_assert(num_threads % TILE_B_COLS == 0);

  const int block_x  = blockIdx.x;
  const int block_y  = blockIdx.y;
  const int thread_x = threadIdx.x;

  // Linear threads are remapped so each thread helps load tiles and then computes a COARSE_Y x
  // COARSE_X output patch.
  const int A_view_y  = thread_x / TILE_A_COLS;
  const int A_view_x  = thread_x % TILE_A_COLS;
  const int B_view_y  = thread_x / TILE_B_COLS;
  const int B_view_x  = thread_x % TILE_B_COLS;
  const int stride_A  = num_threads / TILE_A_COLS;
  const int stride_B  = num_threads / TILE_B_COLS;

  const int row       = COARSE_Y * (thread_x / (TILE_B_COLS / COARSE_X));
  const int col       = COARSE_X * (thread_x % (TILE_B_COLS / COARSE_X));
  const int num_tiles = (K + TILE_A_COLS - 1) / TILE_A_COLS;

  // Shared tiles cover A[block_y*TILE_A_ROWS:(block_y+1)*TILE_A_ROWS,
  // tile*TILE_A_COLS:(tile+1)*TILE_A_COLS] and B[tile*TILE_A_COLS:(tile+1)*TILE_A_COLS,
  // block_x*TILE_B_COLS:(block_x+1)*TILE_B_COLS].
  __shared__ float sh_A[TILE_A_ROWS][TILE_A_COLS];
  __shared__ float sh_B[TILE_A_COLS][TILE_B_COLS];

  float value[COARSE_Y][COARSE_X] = {0.0f};
  float reg_A[COARSE_Y]           = {0.0f};
  float reg_B[COARSE_X]           = {0.0f};

  for (int tile = 0; tile < num_tiles; ++tile) {
    for (int load_offset = 0; load_offset < TILE_A_ROWS; load_offset += stride_A) {
      if (block_y * TILE_A_ROWS + load_offset + A_view_y < M && tile * TILE_A_COLS + A_view_x < K) {
        sh_A[load_offset + A_view_y][A_view_x]
            = A[(block_y * TILE_A_ROWS + load_offset + A_view_y) * K + tile * TILE_A_COLS
                + A_view_x];
      } else {
        sh_A[load_offset + A_view_y][A_view_x] = 0.0f;
      }
    }

    for (int load_offset = 0; load_offset < TILE_A_COLS; load_offset += stride_B) {
      if (tile * TILE_A_COLS + load_offset + B_view_y < K && block_x * TILE_B_COLS + B_view_x < N) {
        sh_B[load_offset + B_view_y][B_view_x] = B[(tile * TILE_A_COLS + B_view_y + load_offset) * N
                                                   + block_x * TILE_B_COLS + B_view_x];
      } else {
        sh_B[load_offset + B_view_y][B_view_x] = 0.0f;
      }
    }
    __syncthreads();

    // Register tiling increases arithmetic intensity: each loaded A/B value is reused across
    // multiple outputs.
    for (int k_tile = 0; k_tile < TILE_A_COLS; ++k_tile) {
      for (int i = 0; i < COARSE_Y; ++i) { reg_A[i] = sh_A[row + i][k_tile]; }
      for (int i = 0; i < COARSE_X; ++i) { reg_B[i] = sh_B[k_tile][col + i]; }

      for (int row_offset = 0; row_offset < COARSE_Y; ++row_offset) {
        for (int col_offset = 0; col_offset < COARSE_X; ++col_offset) {
          value[row_offset][col_offset] += reg_A[row_offset] * reg_B[col_offset];
        }
      }
    }
    __syncthreads();
  }

  // Each thread writes back its COARSE_Y x COARSE_X patch into
  // C[block_y*TILE_A_ROWS+row:block_y*TILE_A_ROWS+row+COARSE_Y,
  //   block_x*TILE_B_COLS+col:block_x*TILE_B_COLS+col+COARSE_X].
  for (int row_offset = 0; row_offset < COARSE_Y; ++row_offset) {
    for (int col_offset = 0; col_offset < COARSE_X; ++col_offset) {
      if (block_y * TILE_A_ROWS + row + row_offset < M
          && block_x * TILE_B_COLS + col + col_offset < N) {
        C[(block_y * TILE_A_ROWS + row + row_offset) * N + block_x * TILE_B_COLS + col + col_offset]
            = value[row_offset][col_offset];
      }
    }
  }
}

void gemm5(float *A, float *B, float *C, int M, int N, int K) {
  // Original parameters before templating: COARSE_X=8, COARSE_Y=8, TILE_A_ROWS=128, TILE_A_COLS=16,
  // TILE_B_COLS=128. Optimized parameters on this machine: COARSE_X=4, COARSE_Y=8, TILE_A_ROWS=64,
  // TILE_A_COLS=16, TILE_B_COLS=128.
  constexpr int COARSE_X    = 4;
  constexpr int COARSE_Y    = 8;
  constexpr int TILE_A_ROWS = 64;
  constexpr int TILE_A_COLS = 16;
  constexpr int TILE_B_COLS = 128;

  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y));
  coarse2D_kernel<COARSE_X, COARSE_Y, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}

template <int COARSE_X, int COARSE_Y, int TILE_A_ROWS, int TILE_A_COLS, int TILE_B_COLS>
__global__ void coarse2Dvec_kernel(float *A, float *B, float *C, int M, int N, int K) {
  constexpr int num_threads = TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y);
  static_assert(num_threads % TILE_A_COLS == 0);
  static_assert(num_threads % TILE_B_COLS == 0);
  static_assert(TILE_A_COLS % 4 == 0);
  static_assert(TILE_B_COLS % 4 == 0);
  static_assert(num_threads <= 1024);
  assert(M % 4 == 0);
  assert(N % 4 == 0);
  assert(K % 4 == 0);

  const int block_y  = blockIdx.y;
  const int block_x  = blockIdx.x;
  const int thread_x = threadIdx.x;

  // The block still computes a TILE_A_ROWS x TILE_B_COLS patch of C, but loads A/B in float4 chunks
  // to reduce instruction count and improve global-memory bandwidth use.
  const int A_view_x  = thread_x % (TILE_A_COLS / 4);
  const int B_view_y  = thread_x / (TILE_B_COLS / 4);
  const int B_view_x  = thread_x % (TILE_B_COLS / 4);
  const int A_view_y  = thread_x / (TILE_A_COLS / 4);

  const int stride_A  = num_threads / (TILE_A_COLS / 4);
  const int stride_B  = num_threads / (TILE_B_COLS / 4);

  const int row       = COARSE_Y * (thread_x / (TILE_B_COLS / COARSE_X));
  const int col       = COARSE_X * (thread_x % (TILE_B_COLS / COARSE_X));
  const int num_tiles = (K + TILE_A_COLS - 1) / TILE_A_COLS;

  // sh_A stores A[block_y*TILE_A_ROWS:(block_y+1)*TILE_A_ROWS,
  // tile*TILE_A_COLS:(tile+1)*TILE_A_COLS] in a transposed shared-memory layout so compute reads
  // become more register-friendly; sh_B stores B[tile*TILE_A_COLS:(tile+1)*TILE_A_COLS,
  // block_x*TILE_B_COLS:(block_x+1)*TILE_B_COLS] directly.
  __shared__ float sh_A[TILE_A_COLS][TILE_A_ROWS];
  __shared__ float sh_B[TILE_A_COLS][TILE_B_COLS];

  float value[COARSE_Y][COARSE_X] = {0.0f};
  float reg_A[COARSE_Y]           = {0.0f};
  float reg_B[COARSE_X]           = {0.0f};

  for (int tile = 0; tile < num_tiles; ++tile) {
    for (int load_offset = 0; load_offset < TILE_A_ROWS; load_offset += stride_A) {
      if (block_y * TILE_A_ROWS + load_offset + A_view_y < M
          && tile * TILE_A_COLS + A_view_x * 4 < K) {
        // Read A[row, k:k+4] as one float4 from global memory, then scatter it into
        // sh_A[tile*TILE_A_COLS+A_view_x*4:tile*TILE_A_COLS+A_view_x*4+4,
        //      block_y*TILE_A_ROWS+load_offset+A_view_y].
        float4 temp_A
            = reinterpret_cast<float4 *>(&A[(block_y * TILE_A_ROWS + load_offset + A_view_y) * K
                                            + tile * TILE_A_COLS + A_view_x * 4])[0];
        sh_A[A_view_x * 4 + 0][load_offset + A_view_y] = temp_A.x;
        sh_A[A_view_x * 4 + 1][load_offset + A_view_y] = temp_A.y;
        sh_A[A_view_x * 4 + 2][load_offset + A_view_y] = temp_A.z;
        sh_A[A_view_x * 4 + 3][load_offset + A_view_y] = temp_A.w;
      } else {
        sh_A[A_view_x * 4 + 0][load_offset + A_view_y] = 0.0f;
        sh_A[A_view_x * 4 + 1][load_offset + A_view_y] = 0.0f;
        sh_A[A_view_x * 4 + 2][load_offset + A_view_y] = 0.0f;
        sh_A[A_view_x * 4 + 3][load_offset + A_view_y] = 0.0f;
      }
    }

    for (int load_offset = 0; load_offset < TILE_A_COLS; load_offset += stride_B) {
      if (tile * TILE_A_COLS + B_view_y + load_offset < K
          && block_x * TILE_B_COLS + B_view_x * 4 < N) {
        // Read B[k, col:col+4] as one float4 from global memory, then place it into
        // sh_B[tile*TILE_A_COLS+B_view_y+load_offset,
        //      block_x*TILE_B_COLS+B_view_x*4:block_x*TILE_B_COLS+B_view_x*4+4].
        float4 temp_B
            = reinterpret_cast<float4 *>(&B[(tile * TILE_A_COLS + B_view_y + load_offset) * N
                                            + block_x * TILE_B_COLS + B_view_x * 4])[0];
        sh_B[B_view_y + load_offset][B_view_x * 4 + 0] = temp_B.x;
        sh_B[B_view_y + load_offset][B_view_x * 4 + 1] = temp_B.y;
        sh_B[B_view_y + load_offset][B_view_x * 4 + 2] = temp_B.z;
        sh_B[B_view_y + load_offset][B_view_x * 4 + 3] = temp_B.w;
      } else {
        sh_B[B_view_y + load_offset][B_view_x * 4 + 0] = 0.0f;
        sh_B[B_view_y + load_offset][B_view_x * 4 + 1] = 0.0f;
        sh_B[B_view_y + load_offset][B_view_x * 4 + 2] = 0.0f;
        sh_B[B_view_y + load_offset][B_view_x * 4 + 3] = 0.0f;
      }
    }
    __syncthreads();

    // Vectorized loads feed the same register-tiled inner loop as GEMM 05, but with less load
    // overhead.
    for (int k_tile = 0; k_tile < TILE_A_COLS; ++k_tile) {
      for (int i = 0; i < COARSE_Y; ++i) { reg_A[i] = sh_A[k_tile][row + i]; }
      for (int i = 0; i < COARSE_X; ++i) { reg_B[i] = sh_B[k_tile][col + i]; }

      for (int row_offset = 0; row_offset < COARSE_Y; ++row_offset) {
        for (int col_offset = 0; col_offset < COARSE_X; ++col_offset) {
          value[row_offset][col_offset] += reg_A[row_offset] * reg_B[col_offset];
        }
      }
    }
    __syncthreads();
  }

  for (int row_offset = 0; row_offset < COARSE_Y; ++row_offset) {
    for (int col_offset = 0; col_offset < COARSE_X; ++col_offset) {
      if (block_y * TILE_A_ROWS + row + row_offset < M
          && block_x * TILE_B_COLS + col + col_offset < N) {
        C[(block_y * TILE_A_ROWS + row + row_offset) * N + block_x * TILE_B_COLS + col + col_offset]
            = value[row_offset][col_offset];
      }
    }
  }
}

void gemm6(float *A, float *B, float *C, int M, int N, int K) {
  // Optimized parameters on this machine: COARSE_X=4, COARSE_Y=8, TILE_A_ROWS=64, TILE_A_COLS=16,
  // TILE_B_COLS=128. Original parameters before templating: COARSE_X=8, COARSE_Y=8,
  // TILE_A_ROWS=128, TILE_A_COLS=16, TILE_B_COLS=128.
  const int COARSE_X = 4, COARSE_Y = 8, TILE_A_ROWS = 64, TILE_A_COLS = 16, TILE_B_COLS = 128;
  constexpr int num_threads = TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y);
  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(num_threads);
  coarse2Dvec_kernel<COARSE_X, COARSE_Y, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}

template <int COARSE_X, int COARSE_Y, int TILE_A_ROWS, int TILE_A_COLS, int TILE_B_COLS>
GemmFn make_gemm6_config() {
  return +[](float *A, float *B, float *C, int M, int N, int K) {
    constexpr int num_threads = TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y);
    dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
    dim3 dim_block(num_threads);
    coarse2Dvec_kernel<COARSE_X, COARSE_Y, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
        <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
  };
}

using Gemm6ConfigLauncher = GemmFn (*)(void);

GemmFn gemm6_config_fn(std::size_t index) {
  constexpr std::array<Gemm6ConfigLauncher, kGemm6Configs.size()> kLaunchers = {
      {
       make_gemm6_config<8, 8, 128, 16, 128>,
       make_gemm6_config<8, 8, 128, 16, 64>,
       make_gemm6_config<8, 4, 128, 16, 128>,
       make_gemm6_config<4, 8, 64, 16, 128>,
       make_gemm6_config<4, 4, 64, 16, 64>,
       }
  };
  return kLaunchers[index]();
}

RunResult run_and_verify(const char *name, GemmFn fn) {
  RunResult result{};

  std::cout << name << "\n";
  std::cout << "Verification Results:\n";
  std::cout << "Warmup runs: " << kWarmupRuns << ", Timed runs: " << kTimedRuns << " (average)\n";
  for (std::size_t index = 0; index < kSizes.size(); ++index) {
    const int size = kSizes[index];

    float *A_device, *B_device, *C_device;
    cudaMalloc((void **)&A_device, size * size * sizeof(float));
    cudaMalloc((void **)&B_device, size * size * sizeof(float));
    cudaMalloc((void **)&C_device, size * size * sizeof(float));

    float *A_host        = new float[size * size];
    float *B_host        = new float[size * size];
    float *C_device_host = new float[size * size];
    float *C_reference   = new float[size * size];

    srand(42);
    for (int j = 0; j < size * size; ++j) {
      A_host[j] = static_cast<float>(rand()) / RAND_MAX;
      B_host[j] = static_cast<float>(rand()) / RAND_MAX;
    }

    cudaMemcpy(A_device, A_host, size * size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(B_device, B_host, size * size * sizeof(float), cudaMemcpyHostToDevice);

    for (int warmup = 0; warmup < kWarmupRuns; ++warmup) {
      fn(A_device, B_device, C_device, size, size, size);
    }
    cudaDeviceSynchronize();

    double total_time = 0.0;
    for (int run = 0; run < kTimedRuns; ++run) {
      auto start = std::chrono::high_resolution_clock::now();
      fn(A_device, B_device, C_device, size, size, size);
      cudaDeviceSynchronize();
      auto end                               = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed  = end - start;
      total_time                            += elapsed.count();
    }
    result.times[index]     = total_time / kTimedRuns;

    const double flop_count = 2.0 * size * size * size;
    result.gflops[index]    = flop_count / (1e9 * result.times[index]);

    cudaMemcpy(C_device_host, C_device, size * size * sizeof(float), cudaMemcpyDeviceToHost);
    const double bytes_moved = 3.0 * size * size * sizeof(float);
    result.bandwidth[index]
        = (result.times[index] > 0) ? (bytes_moved / result.times[index] / 1e8) : 0;

    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        float sum = 0;
        for (int k = 0; k < size; ++k) { sum += A_host[i * size + k] * B_host[k * size + j]; }
        C_reference[i * size + j] = sum;
      }
    }

    bool correct = true;
    for (int i = 0; i < size * size; ++i) {
      if (fabs(C_reference[i] - C_device_host[i]) > 1e-4) {
        correct = false;
        std::cout << "Verification failed at index " << i << ": Host=" << C_reference[i]
                  << ", Device=" << C_device_host[i] << std::endl;
        break;
      }
    }

    std::cout << size << "x" << size << " matrix: ";
    if (correct) {
      std::cout << "\033[32mVerification Successful\033[0m\n";
    } else {
      std::cout << "\033[31mVerification Failed\033[0m\n";
    }

    cudaFree(A_device);
    cudaFree(B_device);
    cudaFree(C_device);
    delete[] A_host;
    delete[] B_host;
    delete[] C_device_host;
    delete[] C_reference;
  }

  std::cout << "\n";
  return result;
}

template <std::size_t N>
void print_metric_table(const std::array<GemmVariant, N> &variants,
                        const std::array<RunResult, N> &results, const char *metric_name,
                        RunResultMetric metric) {
  constexpr int kLabelWidth = 18;
  constexpr int kValueWidth = 22;
  std::cout << "-----------------------------------------------------------------------------------"
               "--------------------"
               "-------\n";
  std::cout << std::left << std::setw(kLabelWidth) << metric_name;
  for (int size : kSizes) { std::cout << std::right << std::setw(kValueWidth) << size; }
  std::cout << "\n---------------------------------------------------------------------------------"
               "--------------------"
               "---------\n";

  for (std::size_t variant_index = 0; variant_index < variants.size(); ++variant_index) {
    std::cout << std::left << std::setw(kLabelWidth) << variants[variant_index].name;
    const auto &values = results[variant_index].*metric;
    for (std::size_t size_index = 0; size_index < values.size(); ++size_index) {
      std::ostringstream cell;
      const double value = values[size_index];
      if (metric == &RunResult::times) {
        cell << std::fixed << std::setprecision(3) << (value * 1e3);
      } else {
        cell << std::fixed << std::setprecision(3) << value;
      }
      std::cout << std::right << std::setw(kValueWidth) << cell.str();
    }
    std::cout << "\n";
  }
  std::cout << "-----------------------------------------------------------------------------------"
               "--------------------"
               "-------\n\n";
}

template <std::size_t N>
void print_speedup_table(const std::array<GemmVariant, N> &variants,
                         const std::array<RunResult, N> &results) {
  constexpr int kLabelWidth = 18;
  constexpr int kValueWidth = 22;
  std::cout << "-----------------------------------------------------------------------------------"
               "--------------------"
               "-------\n";
  std::cout << std::left << std::setw(kLabelWidth) << "Speedup (Time)";
  for (int size : kSizes) { std::cout << std::right << std::setw(kValueWidth) << size; }
  std::cout << "\n---------------------------------------------------------------------------------"
               "--------------------"
               "---------\n";

  for (std::size_t variant_index = 0; variant_index < variants.size(); ++variant_index) {
    std::cout << std::left << std::setw(kLabelWidth) << variants[variant_index].name;
    for (std::size_t size_index = 0; size_index < kSizes.size(); ++size_index) {
      double step_speedup       = 1.0;
      double cumulative_speedup = 1.0;
      const double time_value   = results[variant_index].times[size_index];
      if (variant_index > 0 && time_value != 0.0) {
        step_speedup = results[variant_index - 1].times[size_index] / time_value;
      }
      if (time_value != 0.0) { cumulative_speedup = results[0].times[size_index] / time_value; }

      std::ostringstream cell;
      cell << std::fixed << std::setprecision(2) << step_speedup << "x/" << cumulative_speedup
           << "x";
      std::cout << std::right << std::setw(kValueWidth) << cell.str();
    }
    std::cout << "\n";
  }
  std::cout << "-----------------------------------------------------------------------------------"
               "--------------------"
               "-------\n\n";
}

template <std::size_t N>
void print_gemm6_sweep_summary(const std::array<Gemm6Config, N> &configs,
                               const std::array<RunResult, N> &results) {
  constexpr int kNameWidth                = 22;
  constexpr int kColWidth                 = 18;
  constexpr std::size_t kSummarySizeIndex = kSizes.size() - 1;
  std::size_t best_index                  = 0;
  for (std::size_t i = 1; i < configs.size(); ++i) {
    if (results[i].times[kSummarySizeIndex] < results[best_index].times[kSummarySizeIndex]) {
      best_index = i;
    }
  }

  std::cout << "GEMM 06 Sweep (ranked by 1024x1024 time)\n";
  std::cout << "-----------------------------------------------------------------------------------"
               "-----------\n";
  std::cout << std::left << std::setw(kNameWidth) << "Config" << std::right << std::setw(kColWidth)
            << "1024 ms" << std::setw(kColWidth) << "1024 GFLOPS" << std::setw(kColWidth)
            << "vs base"
            << "\n";
  std::cout << "-----------------------------------------------------------------------------------"
               "-----------\n";

  const double baseline_time = results[0].times[kSummarySizeIndex];
  for (std::size_t i = 0; i < configs.size(); ++i) {
    const double time_ms = results[i].times[kSummarySizeIndex] * 1e3;
    const double speedup = (results[i].times[kSummarySizeIndex] != 0.0)
                             ? baseline_time / results[i].times[kSummarySizeIndex]
                             : 1.0;
    std::ostringstream speedup_cell;
    speedup_cell << std::fixed << std::setprecision(2) << speedup << "x";
    std::cout << std::left << std::setw(kNameWidth) << configs[i].name << std::right
              << std::setw(kColWidth) << std::fixed << std::setprecision(3) << time_ms
              << std::setw(kColWidth) << std::fixed << std::setprecision(3)
              << results[i].gflops[kSummarySizeIndex] << std::setw(kColWidth) << speedup_cell.str()
              << "\n";
  }
  std::cout << "-----------------------------------------------------------------------------------"
               "-----------\n";
  std::cout << "Best GEMM 06 config: " << configs[best_index].name << "  [" << std::fixed
            << std::setprecision(3) << results[best_index].times[kSummarySizeIndex] * 1e3 << " ms, "
            << results[best_index].gflops[kSummarySizeIndex] << " GFLOPS at 1024x1024]\n\n";
}

int main() {
  const VariantList variants = {
      {
       {"GEMM 01", gemm1},
       {"GEMM 02", gemm2},
       {"GEMM 03", gemm3},
       {"GEMM 04", gemm4},
       {"GEMM 05", gemm5},
       {"GEMM 06", gemm6},
       }
  };
  ResultList results{};

  for (std::size_t i = 0; i < variants.size(); ++i) {
    results[i] = run_and_verify(variants[i].name, variants[i].fn);
  }

  std::cout << "Final Summary\n\n";
  print_metric_table(variants, results, "Time (ms)", &RunResult::times);
  print_speedup_table(variants, results);
  print_metric_table(variants, results, "GFLOPS", &RunResult::gflops);
  print_metric_table(variants, results, "Bandwidth (GB/s)", &RunResult::bandwidth);

  Gemm6SweepResults gemm6_sweep_results{};
  for (std::size_t i = 0; i < kGemm6Configs.size(); ++i) {
    gemm6_sweep_results[i] = run_and_verify(kGemm6Configs[i].name, gemm6_config_fn(i));
  }
  print_gemm6_sweep_summary(kGemm6Configs, gemm6_sweep_results);
  return 0;
}
