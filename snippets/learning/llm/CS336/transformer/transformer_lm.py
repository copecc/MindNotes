class TransformerLM:
    def __init__(self, embedding, blocks, norm, lm_head):
        self.embedding = embedding
        self.blocks = blocks
        self.norm = norm
        self.lm_head = lm_head

    def forward(self, token_ids, mask=None):
        x = self.embedding(token_ids)
        for block in self.blocks:
            x = block(x, mask=mask)
        x = self.norm(x)
        return self.lm_head(x)
