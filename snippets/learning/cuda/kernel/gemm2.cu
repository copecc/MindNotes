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
