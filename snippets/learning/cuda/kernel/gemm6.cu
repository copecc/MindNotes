#include "assert.h"

// GEMM 06 -- COARSE 2D VECTORIZED MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
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
  // Optimized parameters on RTX 6000 Ada GPU:
  const int COARSE_X = 4, COARSE_Y = 8, TILE_A_ROWS = 64, TILE_A_COLS = 16, TILE_B_COLS = 128;
  constexpr int num_threads = TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y);
  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(num_threads);
  coarse2Dvec_kernel<COARSE_X, COARSE_Y, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}
