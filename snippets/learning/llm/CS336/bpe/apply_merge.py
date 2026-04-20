def _update_counter(
    self,
    most_frequent_pair: tuple[bytes, bytes],
    token_counter,
    pair_counter,
    pair_indexes,
    merged_bytes: bytes,
):
    affected_indexes = pair_indexes[most_frequent_pair].copy()
    new_pair_counter = Counter()

    for index, index_count in affected_indexes.items():
        if index_count == 0:
            continue

        tokens, count = token_counter[index]
        new_tokens = []
        i = 0
        while i < len(tokens) - 1:
            if (tokens[i], tokens[i + 1]) == most_frequent_pair:
                pair_counter[(tokens[i], tokens[i + 1])] -= count
                pair_indexes[(tokens[i], tokens[i + 1])][index] -= 1

                if i > 0:
                    old_left_pair = (tokens[i - 1], tokens[i])
                    pair_counter[old_left_pair] -= count
                    pair_indexes[old_left_pair][index] -= 1

                    new_left_pair = (tokens[i - 1], merged_bytes)
                    new_pair_counter[new_left_pair] += count
                    pair_indexes[new_left_pair][index] += 1

                if i + 1 < len(tokens) - 1:
                    old_right_pair = (tokens[i + 1], tokens[i + 2])
                    pair_counter[old_right_pair] -= count
                    pair_indexes[old_right_pair][index] -= 1

                    new_right_pair = (merged_bytes, tokens[i + 2])
                    new_pair_counter[new_right_pair] += count
                    pair_indexes[new_right_pair][index] += 1

                new_tokens.append(merged_bytes)
                i += 2
            else:
                new_tokens.append(tokens[i])
                i += 1

        if i < len(tokens):
            new_tokens.append(tokens[i])
        token_counter[index] = (new_tokens, count)

    return new_pair_counter
