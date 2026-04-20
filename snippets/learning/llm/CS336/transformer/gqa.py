import torch


def gqa(
    x: torch.Tensor,
    w_q: torch.Tensor,
    w_k: torch.Tensor,
    w_v: torch.Tensor,
    w_o: torch.Tensor,
    num_q_heads: int,
    num_kv_heads: int,
    cos: torch.Tensor,
    sin: torch.Tensor,
    mask: torch.Tensor | None = None,
) -> torch.Tensor:
    # x: 输入 hidden states，形状为 (batch_size, seq_len, d_model)
    # w_q: query 投影矩阵，形状为 (d_model, d_model)
    # w_k, w_v: key / value 投影矩阵，形状为 (d_model, num_kv_heads * head_dim)
    # w_o: 输出投影矩阵，形状为 (d_model, d_model)
    # num_q_heads: query 的头数
    # num_kv_heads: key / value 的头数
    # cos, sin: RoPE 缓存，形状均为 (seq_len, head_dim)
    # mask: attention mask，形状通常为 (1, 1, seq_len, seq_len)，或可广播到 scores
    # 约束: d_model % num_q_heads == 0，且 num_q_heads % num_kv_heads == 0

    batch_size, seq_len, d_model = x.shape
    head_dim = d_model // num_q_heads
    group_size = num_q_heads // num_kv_heads

    q = x @ w_q
    k = x @ w_k
    v = x @ w_v

    q = q.reshape(batch_size, seq_len, num_q_heads, head_dim).transpose(1, 2)
    k = k.reshape(batch_size, seq_len, num_kv_heads, head_dim).transpose(1, 2)
    v = v.reshape(batch_size, seq_len, num_kv_heads, head_dim).transpose(1, 2)

    cos = cos[None, None, :, :]  # (1, 1, seq_len, head_dim)
    sin = sin[None, None, :, :]  # (1, 1, seq_len, head_dim)

    q_even = q[..., ::2]
    q_odd = q[..., 1::2]
    q_rot = torch.stack((-q_odd, q_even), dim=-1).reshape_as(q)
    q = q * cos + q_rot * sin

    k_even = k[..., ::2]
    k_odd = k[..., 1::2]
    k_rot = torch.stack((-k_odd, k_even), dim=-1).reshape_as(k)
    k = k * cos + k_rot * sin

    k = k[:, :, None, :, :].expand(batch_size, num_kv_heads, group_size, seq_len, head_dim)
    v = v[:, :, None, :, :].expand(batch_size, num_kv_heads, group_size, seq_len, head_dim)
    k = k.reshape(batch_size, num_q_heads, seq_len, head_dim)
    v = v.reshape(batch_size, num_q_heads, seq_len, head_dim)

    scale = head_dim ** -0.5
    scores = q @ k.transpose(-2, -1) * scale  # (batch_size, num_q_heads, seq_len, seq_len)
    if mask is not None:
        scores = scores + mask

    scores = scores - scores.amax(dim=-1, keepdim=True)
    probs = torch.softmax(scores, dim=-1)
    context = probs @ v  # (batch_size, num_q_heads, seq_len, head_dim)

    context = context.transpose(1, 2).reshape(batch_size, seq_len, d_model)
    return context @ w_o  # (batch_size, seq_len, d_model)
