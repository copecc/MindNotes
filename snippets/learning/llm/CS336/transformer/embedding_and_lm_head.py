import torch


def embed_and_project(
    token_ids: torch.Tensor,
    embedding: torch.Tensor,
    final_hidden: torch.Tensor,
) -> tuple[torch.Tensor, torch.Tensor]:
    # token_ids: 输入 token ID，形状为 (batch_size, seq_len)
    # embedding: token embedding 矩阵，形状为 (vocab_size, d_model)
    # final_hidden: 最终 hidden states，形状为 (batch_size, seq_len, d_model)

    input_hidden = embedding[token_ids]          # (batch_size, seq_len, d_model)
    logits = final_hidden @ embedding.T          # (batch_size, seq_len, vocab_size)
    return input_hidden, logits
