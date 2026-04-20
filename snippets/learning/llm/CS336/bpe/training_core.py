import heapq
from collections import Counter, defaultdict


def train(self, input_path: str, desired_num_chunks: int = 8):
    self._init_vocab()

    token_counter = pretokenize_file_to_counter(input_path, self.special_tokens, desired_num_chunks)
    token_counter = [([bytes([b]) for b in token], count) for token, count in token_counter.items()]

    pair_counter = Counter()
    pair_indexes = defaultdict(Counter)

    for index, (tokens, count) in enumerate(token_counter):
        for i in range(len(tokens) - 1):
            pair = (tokens[i], tokens[i + 1])
            pair_counter[pair] += count
            pair_indexes[pair][index] += 1

    heap = [PairItem(token1, token2, count) for (token1, token2), count in pair_counter.items()]
    heapq.heapify(heap)
    merge_count = self.vocab_size - len(self.vocab) - len(self.special_tokens)

    while merge_count > 0:
        most_frequent_item = None
        while heap:
            candidate = heapq.heappop(heap)
            candidate_pair = (candidate.token1, candidate.token2)
            if pair_counter[candidate_pair] == candidate.count:
                most_frequent_item = candidate
                break
            candidate.count = pair_counter[candidate_pair]
            heapq.heappush(heap, candidate)

        if most_frequent_item is None:
            break

        most_frequent_pair = (most_frequent_item.token1, most_frequent_item.token2)
        merged_bytes = most_frequent_pair[0] + most_frequent_pair[1]

        new_pair_counter = self._update_counter(
            most_frequent_pair,
            token_counter,
            pair_counter,
            pair_indexes,
            merged_bytes,
        )

        for new_pair, new_count in new_pair_counter.items():
            pair_counter[new_pair] += new_count
            heapq.heappush(heap, PairItem(new_pair[0], new_pair[1], pair_counter[new_pair]))

        if merged_bytes in self.vocab_set:
            continue

        self.vocab_set.add(merged_bytes)
        self.vocab[len(self.vocab)] = merged_bytes
        self.merges.append(most_frequent_pair)
        merge_count -= 1
