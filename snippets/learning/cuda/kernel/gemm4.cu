// GEMM 04 -- COARSE 1D MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
template <int COARSE_FACTOR, int TILE_A_ROWS, int TILE_A_COLS, int TILE_B_COLS>
__global__ void coarse1D_kernel(float *A, float *B, float *C, int M, int N, int K) {
  const int block_col = blockIdx.x;
  const int block_row = blockIdx.y;
  const int thread_id = threadIdx.x;

  // One block computes the C tile of size TILE_A_ROWS x TILE_B_COLS:
  // C[block_row*TILE_A_ROWS:(block_row+1)*TILE_A_ROWS,
  //   block_col*TILE_B_COLS:(block_col+1)*TILE_B_COLS].

  // Load coordinates contributed by this thread for the current shared-memory tiles.
  const int a_load_row = thread_id / TILE_A_COLS;
  const int a_load_k   = thread_id % TILE_A_COLS;
  const int c_row_group = thread_id / TILE_B_COLS;
  const int thread_col  = thread_id % TILE_B_COLS;
  const int b_load_k    = c_row_group;
  const int b_load_col  = thread_col;

  // This thread accumulates one vertical strip
  // C[c_row0:c_row0+COARSE_FACTOR, c_col].
  const int c_row0    = TILE_A_ROWS * block_row + COARSE_FACTOR * c_row_group;
  const int c_col     = TILE_B_COLS * block_col + thread_col;
  const int num_tiles = (K + TILE_A_COLS - 1) / TILE_A_COLS;

  // For each tile in K:
  // sh_A[row_in_tile][k_in_tile] <- A[block_row tile, current K tile]
  // sh_B[k_in_tile][col_in_tile] <- B[current K tile, block_col tile]
  __shared__ float sh_A[TILE_A_ROWS][TILE_A_COLS];
  __shared__ float sh_B[TILE_A_COLS][TILE_B_COLS];

  float accum[COARSE_FACTOR] = {0.0f};
  for (int tile = 0; tile < num_tiles; ++tile) {
    // This thread loads one A element:
    // global A[block_row*TILE_A_ROWS + a_load_row, tile*TILE_A_COLS + a_load_k]
    //   -> sh_A[a_load_row][a_load_k].
    if (block_row * TILE_A_ROWS + a_load_row < M && tile * TILE_A_COLS + a_load_k < K) {
      sh_A[a_load_row][a_load_k]
          = A[(block_row * TILE_A_ROWS + a_load_row) * K + tile * TILE_A_COLS + a_load_k];
    } else {
      sh_A[a_load_row][a_load_k] = 0.0f;
    }

    // This thread loads one B element:
    // global B[tile*TILE_A_COLS + b_load_k, block_col*TILE_B_COLS + b_load_col]
    //   -> sh_B[b_load_k][b_load_col].
    if (tile * TILE_A_COLS + b_load_k < K && block_col * TILE_B_COLS + b_load_col < N) {
      sh_B[b_load_k][b_load_col]
          = B[(tile * TILE_A_COLS + b_load_k) * N + block_col * TILE_B_COLS + b_load_col];
    } else {
      sh_B[b_load_k][b_load_col] = 0.0f;
    }
    __syncthreads();

    // One shared B value is reused across COARSE_FACTOR output rows from the same thread.
    for (int k_tile = 0; k_tile < TILE_A_COLS; ++k_tile) {
      const float b_val = sh_B[k_tile][thread_col];
      for (int c = 0; c < COARSE_FACTOR; ++c) {
        accum[c] += sh_A[c_row_group * COARSE_FACTOR + c][k_tile] * b_val;
      }
    }
    __syncthreads();
  }

  for (int c = 0; c < COARSE_FACTOR; ++c) {
    if (c_row0 + c < M && c_col < N) { C[(c_row0 + c) * N + c_col] = accum[c]; }
  }
}

void gemm4(float *A, float *B, float *C, int M, int N, int K) {
  constexpr int COARSE_FACTOR = 8, TILE_A_ROWS = 64, TILE_A_COLS = 8, TILE_B_COLS = 64;

  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(TILE_A_ROWS * TILE_B_COLS / COARSE_FACTOR);
  coarse1D_kernel<COARSE_FACTOR, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}
