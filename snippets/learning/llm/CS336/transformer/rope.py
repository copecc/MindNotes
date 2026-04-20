import torch


def build_rope_cache(
    seq_len: int,
    head_dim: int,
    base: float = 10000.0,
) -> tuple[torch.Tensor, torch.Tensor]:
    # seq_len: 位置数
    # head_dim: 单个 attention head 的维度，通常要求为偶数
    # base: RoPE 的频率底数

    positions = torch.arange(seq_len, dtype=torch.float32)  # (seq_len,)
    freq_indices = torch.arange(0, head_dim, 2, dtype=torch.float32)  # (head_dim / 2,)
    theta = base ** (-freq_indices / head_dim)  # (head_dim / 2,)

    angles = torch.outer(positions, theta)  # (seq_len, head_dim / 2)
    cos = torch.repeat_interleave(torch.cos(angles), 2, dim=-1)  # (seq_len, head_dim)
    sin = torch.repeat_interleave(torch.sin(angles), 2, dim=-1)  # (seq_len, head_dim)
    return cos, sin


def apply_rope(x: torch.Tensor, cos: torch.Tensor, sin: torch.Tensor) -> torch.Tensor:
    # x: (..., head_dim)
    # cos, sin: 与 x 最后一维对齐、且可 broadcast 的 RoPE 缓存
    x_even = x[..., ::2]
    x_odd = x[..., 1::2]
    # 把 [x0, x1, x2, x3, ...] 按二维对子重排成 [-x1, x0, -x3, x2, ...]
    x_rot = torch.stack((-x_odd, x_even), dim=-1).reshape_as(x)

    return x * cos + x_rot * sin
