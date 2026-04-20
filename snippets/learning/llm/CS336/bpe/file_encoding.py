import multiprocessing as mp
import numpy as np


def encode_file(input_path: str, tokenizer, output_file: str, use_memmap: bool = False, desired_num_chunks: int = 8):
    with open(input_path, "rb") as f:
        boundaries = find_chunk_boundaries(f, desired_num_chunks, b"<|endoftext|>")

    chunk_tasks = [(input_path, start, end, tokenizer) for start, end in zip(boundaries[:-1], boundaries[1:])]
    with mp.Pool(processes=min(desired_num_chunks, 4 * mp.cpu_count() // 5)) as pool:
        results = pool.starmap(tokenize_chunk, chunk_tasks)

    total_tokens = sum(len(result) for result in results)
    token_array = np.memmap(output_file, np.uint16, "w+", shape=(total_tokens,)) if use_memmap else np.empty(total_tokens, dtype=np.uint16)

    offset = 0
    for result in results:
        token_array[offset : offset + len(result)] = result
        offset += len(result)


def encode_file_streaming(input_path: str, tokenizer, output_file: str, desired_MB_per_chunk: int = 200):
    with open(input_path, "rb") as f:
        f.seek(0, os.SEEK_END)
        file_size = f.tell()
        f.seek(0)
        desired_num_chunks = max(1, file_size // (desired_MB_per_chunk * 1024 * 1024))
        boundaries = find_chunk_boundaries(f, desired_num_chunks, b"<|endoftext|>")

    chunk_tasks = [(input_path, start, end, tokenizer) for start, end in zip(boundaries[:-1], boundaries[1:])]
