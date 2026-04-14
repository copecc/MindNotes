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
