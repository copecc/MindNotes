#include <cuda_runtime.h>

// 融合因果掩码的点积计算
// 仅计算 j <= i 的部分，其他部分填充极大负值
__global__ void causal_mask_fusion_basic_kernel(
    const float* q, const float* k, float* scores, 
    int seq_len, int head_dim, float scale) {
    
    int row = blockIdx.y * blockDim.y + threadIdx.y; // i (query)
    int col = blockIdx.x * blockDim.x + threadIdx.x; // j (key)
    
    if (row < seq_len && col < seq_len) {
        if (col > row) {
            // 因果掩码：不能看到未来的信息
            scores[row * seq_len + col] = -1e20f;
        } else {
            // 进行正常的 Q*K 点积
            float sum = 0.0f;
            for (int d = 0; d < head_dim; ++d) {
                sum += q[row * head_dim + d] * k[col * head_dim + d];
            }
            scores[row * seq_len + col] = sum * scale;
        }
    }
}
