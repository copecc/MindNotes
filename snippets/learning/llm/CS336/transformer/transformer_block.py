class TransformerBlock:
    def __init__(self, attn, ffn, norm1, norm2):
        self.attn = attn
        self.ffn = ffn
        self.norm1 = norm1
        self.norm2 = norm2

    def __call__(self, x, mask=None):
        x = x + self.attn(self.norm1(x), mask=mask)
        x = x + self.ffn(self.norm2(x))
        return x
