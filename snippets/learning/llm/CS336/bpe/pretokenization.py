import regex as re

PAT = r"""'(?:[sdmt]|ll|ve|re)| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+"""


def pretokenize_iter(text: str, special_tokens: list[str]):
    special_tokens = sorted(special_tokens, key=len, reverse=True)
    special_pattern = re.compile(f'({"|".join(map(re.escape, special_tokens))})')
    split_pattern = re.compile(PAT)

    for segment in special_pattern.split(text):
        if segment in special_tokens:
            yield segment.encode("utf-8")
            continue

        for match in split_pattern.finditer(segment):
            if piece := match.group():
                yield piece.encode("utf-8")
