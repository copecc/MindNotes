#include <cuda_runtime.h>

// Rotary Position Embedding 基础实现
// 通常每个元素相邻两维打包形成 (x0, x1)，进行 2D 旋转
__global__ void rope_embedding_basic_kernel(float* q, float* k, const float* cos_cache, const float* sin_cache, int seq_len, int head_dim) {
    // blockIdx.x 代表 token 位置 (seq_id)
    // threadIdx.x 代表特征维度的打包组编号，范围为 [0, head_dim / 2 - 1]
    int seq_id = blockIdx.x;
    int group_id = threadIdx.x; 

    if (seq_id < seq_len && group_id < head_dim / 2) {
        // 计算在 q 和 k 数组中的基础偏移
        int offset = seq_id * head_dim + group_id * 2;
        
        // 缓存获取当前组对应的旋转角度余弦和正弦值
        // 假设 cos/sin table 按照 seq_len * (head_dim/2) 排列
        int cache_idx = seq_id * (head_dim / 2) + group_id;
        float cos_val = cos_cache[cache_idx];
        float sin_val = sin_cache[cache_idx];

        // 提取 Query 的数值
        float q_0 = q[offset];
        float q_1 = q[offset + 1];
        // 提取 Key 的数值
        float k_0 = k[offset];
        float k_1 = k[offset + 1];

        // 执行 2D 平面旋转操作写回
        q[offset]     = q_0 * cos_val - q_1 * sin_val;
        q[offset + 1] = q_1 * cos_val + q_0 * sin_val;

        k[offset]     = k_0 * cos_val - k_1 * sin_val;
        k[offset + 1] = k_1 * cos_val + k_0 * sin_val;
    }
}
