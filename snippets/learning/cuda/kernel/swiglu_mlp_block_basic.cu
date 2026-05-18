#include <cuda_runtime.h>
#include <math.h>

// 融合的 SwiGLU 放缩激活
// Y = (X * W1) * SiLU(X * W2) 
// 为了演示简化，假设 W1 结果为 gate, W2 结果为 up
__global__ void swiglu_mlp_block_basic_kernel(
    const float* gate, const float* up, float* down, int N) {
    
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx < N) {
        float gate_val = gate[idx];
        float up_val   = up[idx];
        
        // SiLU 计算: x * sigmoid(x)
        float silu_val = gate_val / (1.0f + expf(-gate_val));
        
        // 门控逻辑结合
        down[idx] = silu_val * up_val;
    }
}
