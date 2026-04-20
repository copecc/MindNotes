import torch


@torch.no_grad()
def adam_step(param_groups, state):
    for group in param_groups:
        lr = group['lr']
        beta1, beta2 = group['betas']
        eps = group['eps']

        for p in group['params']:
            if p.grad is None:
                continue

            grad = p.grad
            if grad.is_sparse:
                raise RuntimeError('Adam does not support sparse gradients')

            if p not in state:
                state[p] = {
                    'step': 0,
                    'exp_avg': torch.zeros_like(p),
                    'exp_avg_sq': torch.zeros_like(p),
                }

            param_state = state[p]
            param_state['step'] += 1
            exp_avg = param_state['exp_avg']
            exp_avg_sq = param_state['exp_avg_sq']
            step = param_state['step']

            exp_avg.mul_(beta1).add_(grad, alpha=1 - beta1)
            exp_avg_sq.mul_(beta2).addcmul_(grad, grad, value=1 - beta2)

            bias_correction1 = 1 - beta1 ** step
            bias_correction2 = 1 - beta2 ** step
            exp_avg_hat = exp_avg / bias_correction1
            exp_avg_sq_hat = exp_avg_sq / bias_correction2
            denom = exp_avg_sq_hat.sqrt().add_(eps)

            p.addcdiv_(exp_avg_hat, denom, value=-lr)
