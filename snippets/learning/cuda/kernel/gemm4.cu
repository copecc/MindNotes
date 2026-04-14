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
  constexpr int COARSE_FACTOR = 8, TILE_A_ROWS = 64, TILE_A_COLS = 8, TILE_B_COLS = 64;

  dim3 dim_grid((N + TILE_B_COLS - 1) / TILE_B_COLS, (M + TILE_A_ROWS - 1) / TILE_A_ROWS);
  dim3 dim_block(TILE_A_ROWS * TILE_B_COLS / COARSE_FACTOR);
  coarse1D_kernel<COARSE_FACTOR, TILE_A_ROWS, TILE_A_COLS, TILE_B_COLS>
      <<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}
