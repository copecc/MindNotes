def bytes_to_unicode() -> dict[int, str]:
    # 这些区间本身就是可打印字符，直接保留可减少词表文件中的不可见符号。
    bs = list(range(ord("!"), ord("~") + 1))
    bs += list(range(ord("¡"), ord("¬") + 1))
    bs += list(range(ord("®"), ord("ÿ") + 1))

    cs = bs[:]
    extra = 0
    for b in range(256):
        if b in bs:
            continue
        # 其余 byte 映射到额外 code point，保证 256 个 byte 都有唯一可见表示。
        bs.append(b)
        cs.append(256 + extra)
        extra += 1

    return {byte: chr(codepoint) for byte, codepoint in zip(bs, cs)}


mapping = bytes_to_unicode()
visible = "".join(mapping[b] for b in b"hello\n")
