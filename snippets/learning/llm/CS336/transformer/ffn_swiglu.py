import torch
import torch.nn.functional as F


def swiglu(x: torch.Tensor, w_gate: torch.Tensor, w_up: torch.Tensor, w_down: torch.Tensor) -> torch.Tensor:
    gate = F.silu(x @ w_gate)
    up = x @ w_up
    return (gate * up) @ w_down
