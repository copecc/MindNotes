#include <cuda_runtime.h>

// 分组查询注意力 GQA: 多个 Query Head 共享同一个 K, V Head
__global__ void grouped_query_attention_basic_kernel(
    const float* q, const float* k, float* scores,
    int num_q_heads, int num_kv_heads, int head_dim, int seq_len) {
    
    // GQA 下，每个 KV head 对应 num_q_heads / num_kv_heads 个 Q head (group size)
    int group_size = num_q_heads / num_kv_heads;

    int q_head_idx = blockIdx.y; // Query Head 索引
    int kv_head_idx = q_head_idx / group_size; // 寻址对应的共享 KV Head 索引

    int q_seq_idx = blockIdx.x; // Query 的位置
    int k_seq_idx = threadIdx.x; // Key 的位置

    if (k_seq_idx < seq_len && q_seq_idx < seq_len) {
        float score = 0.0f;
        for (int d = 0; d < head_dim; d++) {
            float q_val = q[(q_seq_idx * num_q_heads + q_head_idx) * head_dim + d];
            // 多个 Q Head 物理上访问读取相同 KV 头上的数据，实现显存复用
            float k_val = k[(k_seq_idx * num_kv_heads + kv_head_idx) * head_dim + d];
            score += q_val * k_val;
        }
        
        scores[(q_head_idx * seq_len + q_seq_idx) * seq_len + k_seq_idx] = score;
    }
}
