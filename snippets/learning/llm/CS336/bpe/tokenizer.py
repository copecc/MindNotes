class BPETokenizer:
    def __init__(self, vocab: dict[int, bytes], merges: list[tuple[bytes, bytes]], special_tokens=None):
        self.vocab = vocab
        self.special_tokens = special_tokens or []
        self.byte_to_token_id = {value: key for key, value in vocab.items()}
        self.merge_rules = {(left, right): rank for rank, (left, right) in enumerate(merges)}
        self.cache = {}

    def _apply_bpe(self, token: bytes) -> list[int]:
        if len(token) <= 1:
            return [self.byte_to_token_id[token]]
        if token in self.cache:
            return self.cache[token]

        token_bytes = [bytes([b]) for b in token]
        while True:
            candidates = [
                (self.merge_rules[pair], i)
                for i in range(len(token_bytes) - 1)
                if (pair := (token_bytes[i], token_bytes[i + 1])) in self.merge_rules
            ]
            if not candidates:
                break

            best_rank = min(rank for rank, _ in candidates)
            merge_indexes = {i for rank, i in candidates if rank == best_rank}

            new_token_bytes = []
            i = 0
            while i < len(token_bytes):
                if i in merge_indexes:
                    new_token_bytes.append(token_bytes[i] + token_bytes[i + 1])
                    i += 2
                else:
                    new_token_bytes.append(token_bytes[i])
                    i += 1
            token_bytes = new_token_bytes

        token_ids = [self.byte_to_token_id[piece] for piece in token_bytes]
        self.cache[token] = token_ids
        return token_ids

    def encode(self, text: str) -> list[int]:
        token_ids = []
        for token in pretokenize_text(text, self.special_tokens):
            if token in self.byte_to_token_id:
                token_ids.append(self.byte_to_token_id[token])
            else:
                token_ids.extend(self._apply_bpe(token))
        return token_ids

    def decode(self, ids: list[int]) -> str:
        return b"".join(self.vocab[token_id] for token_id in ids).decode("utf-8", errors="replace")
