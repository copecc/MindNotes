import torch


def split_heads(x: torch.Tensor, num_heads: int) -> torch.Tensor:
    batch, seq_len, d_model = x.shape
    head_dim = d_model // num_heads
    return x.reshape(batch, seq_len, num_heads, head_dim).transpose(1, 2)


def merge_heads(x: torch.Tensor) -> torch.Tensor:
    batch, num_heads, seq_len, head_dim = x.shape
    return x.transpose(1, 2).reshape(batch, seq_len, num_heads * head_dim)


def repeat_kv(x: torch.Tensor, num_q_heads: int) -> torch.Tensor:
    batch, num_kv_heads, seq_len, head_dim = x.shape
    group_size = num_q_heads // num_kv_heads
    x = x[:, :, None, :, :].expand(batch, num_kv_heads, group_size, seq_len, head_dim)
    return x.reshape(batch, num_q_heads, seq_len, head_dim)


def fused_qkv_projection(x: torch.Tensor, w_qkv: torch.Tensor) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    qkv = x @ w_qkv
    return torch.chunk(qkv, 3, dim=-1)


def scaled_dot_product_attention(
    q: torch.Tensor,
    k: torch.Tensor,
    v: torch.Tensor,
    mask: torch.Tensor | None = None,
) -> torch.Tensor:
    scale = q.shape[-1] ** -0.5
    scores = q @ k.transpose(-2, -1) * scale
    if mask is not None:
        scores = scores + mask
    scores = scores - scores.amax(dim=-1, keepdim=True)
    probs = torch.softmax(scores, dim=-1)
    return probs @ v


def mha(
    x: torch.Tensor,
    w_q: torch.Tensor,
    w_k: torch.Tensor,
    w_v: torch.Tensor,
    w_o: torch.Tensor,
    num_heads: int,
    cos: torch.Tensor,
    sin: torch.Tensor,
    mask: torch.Tensor | None = None,
) -> torch.Tensor:
    from rope import apply_rope

    q = split_heads(x @ w_q, num_heads)
    k = split_heads(x @ w_k, num_heads)
    v = split_heads(x @ w_v, num_heads)

    cos = cos[None, None, :, :]
    sin = sin[None, None, :, :]
    q = apply_rope(q, cos, sin)
    k = apply_rope(k, cos, sin)

    attn = scaled_dot_product_attention(q, k, v, mask=mask)
    return merge_heads(attn) @ w_o


def mha_fused_qkv(
    x: torch.Tensor,
    w_qkv: torch.Tensor,
    w_o: torch.Tensor,
    num_heads: int,
    cos: torch.Tensor,
    sin: torch.Tensor,
    mask: torch.Tensor | None = None,
) -> torch.Tensor:
    from rope import apply_rope

    q, k, v = fused_qkv_projection(x, w_qkv)
    q = split_heads(q, num_heads)
    k = split_heads(k, num_heads)
    v = split_heads(v, num_heads)

    cos = cos[None, None, :, :]
    sin = sin[None, None, :, :]
    q = apply_rope(q, cos, sin)
    k = apply_rope(k, cos, sin)

    attn = scaled_dot_product_attention(q, k, v, mask=mask)
    return merge_heads(attn) @ w_o


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
    from rope import apply_rope

    q = split_heads(x @ w_q, num_q_heads)
    k = split_heads(x @ w_k, num_kv_heads)
    v = split_heads(x @ w_v, num_kv_heads)

    cos = cos[None, None, :, :]
    sin = sin[None, None, :, :]
    q = apply_rope(q, cos, sin)
    k = apply_rope(k, cos, sin)

    k = repeat_kv(k, num_q_heads)
    v = repeat_kv(v, num_q_heads)

    attn = scaled_dot_product_attention(q, k, v, mask=mask)
    return merge_heads(attn) @ w_o
