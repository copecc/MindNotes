import torch


def append_kv(
    cache_k: torch.Tensor,
    cache_v: torch.Tensor,
    new_k: torch.Tensor,
    new_v: torch.Tensor,
) -> tuple[torch.Tensor, torch.Tensor]:
    cache_k = torch.cat([cache_k, new_k], dim=-2)
    cache_v = torch.cat([cache_v, new_v], dim=-2)
    return cache_k, cache_v
