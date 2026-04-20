import torch


def softmax(x: torch.Tensor, dim: int = -1) -> torch.Tensor:
    x = x - x.amax(dim=dim, keepdim=True)
    exp_x = torch.exp(x)
    return exp_x / exp_x.sum(dim=dim, keepdim=True)
