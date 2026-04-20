def attention_flops(seq_len: int, d_model: int) -> int:
    return 4 * seq_len * seq_len * d_model


def mlp_flops(seq_len: int, d_model: int, d_ff: int) -> int:
    return 2 * seq_len * d_model * d_ff
