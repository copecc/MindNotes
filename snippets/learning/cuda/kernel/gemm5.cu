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
  // Original parameters : COARSE_X=8, COARSE_Y=8, TILE_A_ROWS=128, TILE_A_COLS=16,
  // TILE_B_COLS=128.
  // Optimized parameters on RTX 6000 Ada GPU:
  constexpr int COARSE_X = 4, COARSE_Y = 8, TILE_A_ROWS = 64, TILE_A_COLS = 16, TILE_B_COLS = 128;

  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(TILE_A_ROWS * TILE_B_COLS / (COARSE_X * COARSE_Y));
  coarse2D_kernel<COARSE_X, COARSE_Y, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}
