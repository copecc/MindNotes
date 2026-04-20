import torch


def attention(q: torch.Tensor, k: torch.Tensor, v: torch.Tensor, mask: torch.Tensor | None = None) -> torch.Tensor:
    scores = q @ k.transpose(-2, -1) / torch.sqrt(torch.tensor(q.shape[-1], device=q.device, dtype=q.dtype))
    if mask is not None:
        scores = scores + mask
    probs = torch.softmax(scores, dim=-1)
    return probs @ v
