// GEMM 03 -- TILED MAT_MUL IMPLEMENTATION
// SGEMM is C = α*(A @ B)+β*C; here α=1, β=0
template <int TILE_WIDTH>
__global__ void tiled_kernel(float *A, float *B, float *C, int M, int N, int K) {
  assert(TILE_WIDTH == blockDim.x);
  assert(TILE_WIDTH == blockDim.y);

  const int block_col = blockIdx.x;
  const int block_row = blockIdx.y;
  const int thread_col = threadIdx.x;
  const int thread_row = threadIdx.y;

  // One block computes the C tile
  // C[block_row*TILE_WIDTH:(block_row+1)*TILE_WIDTH,
  //   block_col*TILE_WIDTH:(block_col+1)*TILE_WIDTH].
  // One thread computes one output element C[c_row, c_col].
  const int c_row     = TILE_WIDTH * block_row + thread_row;
  const int c_col     = TILE_WIDTH * block_col + thread_col;
  const int num_tiles = (K + TILE_WIDTH - 1) / TILE_WIDTH;

  // For each tile in K:
  // sh_A[row_in_tile][k_in_tile] <- A[block_row tile, current K tile]
  // sh_B[k_in_tile][col_in_tile] <- B[current K tile, block_col tile]
  __shared__ float sh_A[TILE_WIDTH][TILE_WIDTH];
  __shared__ float sh_B[TILE_WIDTH][TILE_WIDTH];

  float accum = 0.0f;
  for (int tile = 0; tile < num_tiles; ++tile) {
    // This thread loads one A element:
    // global A[c_row, tile*TILE_WIDTH + thread_col]
    //   -> sh_A[thread_row][thread_col].
    if (c_row < M && tile * TILE_WIDTH + thread_col < K) {
      sh_A[thread_row][thread_col] = A[c_row * K + tile * TILE_WIDTH + thread_col];
    } else {
      sh_A[thread_row][thread_col] = 0.0f;
    }

    // This thread loads one B element:
    // global B[tile*TILE_WIDTH + thread_row, c_col]
    //   -> sh_B[thread_row][thread_col].
    if (tile * TILE_WIDTH + thread_row < K && c_col < N) {
      sh_B[thread_row][thread_col] = B[(tile * TILE_WIDTH + thread_row) * N + c_col];
    } else {
      sh_B[thread_row][thread_col] = 0.0f;
    }

    __syncthreads();

    // Shared-memory tiles are reused by all threads in the block across this K slice.
    for (int k_tile = 0; k_tile < TILE_WIDTH; ++k_tile) {
      accum += sh_A[thread_row][k_tile] * sh_B[k_tile][thread_col];
    }
    __syncthreads();
  }

  if (c_row < M && c_col < N) { C[c_row * N + c_col] = accum; }
}

void gemm3(float *A, float *B, float *C, int M, int N, int K) {
  constexpr int TILE_WIDTH = 32;

  dim3 dim_block(TILE_WIDTH, TILE_WIDTH, 1);
  dim3 dim_grid((M + TILE_WIDTH - 1) / TILE_WIDTH, (N + TILE_WIDTH - 1) / TILE_WIDTH, 1);
  tiled_kernel<TILE_WIDTH><<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}
