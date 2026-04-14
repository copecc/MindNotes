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
  constexpr int TILE_WIDTH = 32;

  dim3 dim_block(TILE_WIDTH, TILE_WIDTH, 1);
  dim3 dim_grid((M + TILE_WIDTH - 1) / TILE_WIDTH, (N + TILE_WIDTH - 1) / TILE_WIDTH, 1);
  tiled_kernel<TILE_WIDTH><<<dim_grid, dim_block>>>(A, B, C, M, N, K);
}
