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

  const int block_row = blockIdx.y;
  const int block_col = blockIdx.x;
  const int thread_id = threadIdx.x;

  // One block computes the C tile
  // C[block_row*TILE_A_ROWS:(block_row+1)*TILE_A_ROWS,
  //   block_col*TILE_B_COLS:(block_col+1)*TILE_B_COLS].
  // A and B are loaded in float4 chunks to reduce the number of global-memory load instructions.

  const int a_vec_k    = thread_id % (TILE_A_COLS / 4);
  const int a_load_row = thread_id / (TILE_A_COLS / 4);
  const int b_load_k   = thread_id / (TILE_B_COLS / 4);
  const int b_vec_col  = thread_id % (TILE_B_COLS / 4);

  const int a_load_row_stride = num_threads / (TILE_A_COLS / 4);
  const int b_load_k_stride   = num_threads / (TILE_B_COLS / 4);

  // This thread accumulates a COARSE_Y x COARSE_X patch whose top-left corner is
  // (c_row_in_tile, c_col_in_tile) inside the block tile.
  const int c_row_in_tile = COARSE_Y * (thread_id / (TILE_B_COLS / COARSE_X));
  const int c_col_in_tile = COARSE_X * (thread_id % (TILE_B_COLS / COARSE_X));
  const int num_tiles = (K + TILE_A_COLS - 1) / TILE_A_COLS;

  // For each tile in K:
  // sh_A[k_in_tile][row_in_tile] <- A[block_row tile, current K tile]
  // sh_B[k_in_tile][col_in_tile] <- B[current K tile, block_col tile]
  // sh_A is stored as [k][row] instead of [row][k] so the compute loop can read sh_A[k_tile][row + i]
  // directly.
  __shared__ float sh_A[TILE_A_COLS][TILE_A_ROWS];
  __shared__ float sh_B[TILE_A_COLS][TILE_B_COLS];

  float accum[COARSE_Y][COARSE_X] = {0.0f};
  float a_frag[COARSE_Y]          = {0.0f};
  float b_frag[COARSE_X]          = {0.0f};

  for (int tile = 0; tile < num_tiles; ++tile) {
    // Fill sh_A for the current K tile.
    // Each thread reads one float4 from
    // A[block_row*TILE_A_ROWS + row_in_tile, tile*TILE_A_COLS + 4*a_vec_k : +4]
    // and scatters it into sh_A[k_in_tile][row_in_tile].
    for (int a_row_base = 0; a_row_base < TILE_A_ROWS; a_row_base += a_load_row_stride) {
      const int row_in_tile = a_row_base + a_load_row;
      if (block_row * TILE_A_ROWS + row_in_tile < M && tile * TILE_A_COLS + a_vec_k * 4 < K) {
        float4 temp_A
            = reinterpret_cast<float4 *>(&A[(block_row * TILE_A_ROWS + row_in_tile) * K
                                            + tile * TILE_A_COLS + a_vec_k * 4])[0];
        sh_A[a_vec_k * 4 + 0][row_in_tile] = temp_A.x;
        sh_A[a_vec_k * 4 + 1][row_in_tile] = temp_A.y;
        sh_A[a_vec_k * 4 + 2][row_in_tile] = temp_A.z;
        sh_A[a_vec_k * 4 + 3][row_in_tile] = temp_A.w;
      } else {
        sh_A[a_vec_k * 4 + 0][row_in_tile] = 0.0f;
        sh_A[a_vec_k * 4 + 1][row_in_tile] = 0.0f;
        sh_A[a_vec_k * 4 + 2][row_in_tile] = 0.0f;
        sh_A[a_vec_k * 4 + 3][row_in_tile] = 0.0f;
      }
    }

    // Fill sh_B for the current K tile.
    // Each thread reads one float4 from
    // B[tile*TILE_A_COLS + k_in_tile, block_col*TILE_B_COLS + 4*b_vec_col : +4]
    // and stores it into sh_B[k_in_tile][col_in_tile].
    for (int b_k_base = 0; b_k_base < TILE_A_COLS; b_k_base += b_load_k_stride) {
      const int k_in_tile = b_k_base + b_load_k;
      if (tile * TILE_A_COLS + k_in_tile < K && block_col * TILE_B_COLS + b_vec_col * 4 < N) {
        float4 temp_B
            = reinterpret_cast<float4 *>(&B[(tile * TILE_A_COLS + k_in_tile) * N
                                            + block_col * TILE_B_COLS + b_vec_col * 4])[0];
        sh_B[k_in_tile][b_vec_col * 4 + 0] = temp_B.x;
        sh_B[k_in_tile][b_vec_col * 4 + 1] = temp_B.y;
        sh_B[k_in_tile][b_vec_col * 4 + 2] = temp_B.z;
        sh_B[k_in_tile][b_vec_col * 4 + 3] = temp_B.w;
      } else {
        sh_B[k_in_tile][b_vec_col * 4 + 0] = 0.0f;
        sh_B[k_in_tile][b_vec_col * 4 + 1] = 0.0f;
        sh_B[k_in_tile][b_vec_col * 4 + 2] = 0.0f;
        sh_B[k_in_tile][b_vec_col * 4 + 3] = 0.0f;
      }
    }
    __syncthreads();

    // The vectorized loads feed the same register-tiled inner loop as GEMM 05.
    for (int k_tile = 0; k_tile < TILE_A_COLS; ++k_tile) {
      for (int i = 0; i < COARSE_Y; ++i) { a_frag[i] = sh_A[k_tile][c_row_in_tile + i]; }
      for (int i = 0; i < COARSE_X; ++i) { b_frag[i] = sh_B[k_tile][c_col_in_tile + i]; }

      for (int row_offset = 0; row_offset < COARSE_Y; ++row_offset) {
        for (int col_offset = 0; col_offset < COARSE_X; ++col_offset) {
          accum[row_offset][col_offset] += a_frag[row_offset] * b_frag[col_offset];
        }
      }
    }
    __syncthreads();
  }

  // Write back the thread-local COARSE_Y x COARSE_X patch.
  for (int row_offset = 0; row_offset < COARSE_Y; ++row_offset) {
    for (int col_offset = 0; col_offset < COARSE_X; ++col_offset) {
      if (block_row * TILE_A_ROWS + c_row_in_tile + row_offset < M
          && block_col * TILE_B_COLS + c_col_in_tile + col_offset < N) {
        C[(block_row * TILE_A_ROWS + c_row_in_tile + row_offset) * N + block_col * TILE_B_COLS
          + c_col_in_tile + col_offset] = accum[row_offset][col_offset];
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
