#!/usr/bin/env python3
"""Build-local exact pack; do not publish the generated coefficient bytes."""
import re
import sys
from pathlib import Path

def pack(source):
    words = [int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{8})u", source)]
    if len(words) != 513:
        raise ValueError("expected the exact 513 Miami words")
    lane = words
    mask = 0xffffffff
    for _ in range(3):
        lane = [(u - p) & mask for p, u in zip([0] + lane, lane)]
    signed = [v if v < 0x80000000 else v - (1 << 32) for v in lane]
    code = [(v << 1) ^ (v >> 31) for v in signed]
    buf = bytearray()
    for i in range(0, len(code), 8):
        group = code[i:i + 8]
        width = max(group).bit_length()
        data = sum(v << (j * width) for j, v in enumerate(group))
        buf.append(width)
        buf.extend(data.to_bytes((len(group) * width + 7) // 8, 'little'))
    check = []
    offset = 0
    a = b = c = 0
    for i in range(0, len(code), 8):
        width = buf[offset]
        n = min(8, len(code) - i)
        count = (n * width + 7) // 8
        data = int.from_bytes(buf[offset + 1:offset + 1 + count], 'little')
        offset += 1 + count
        for j in range(n):
            u = (data >> (j * width)) & ((1 << width) - 1)
            a = (a + ((u >> 1) ^ -(u & 1))) & mask
            b = (b + a) & mask
            c = (c + b) & mask
            check.append(c)
    if check != words or offset != len(buf):
        raise ValueError("Miami table failed exact recomposition")
    return buf

def generate(src, out):
    source = src.read_text()
    data = pack(source)
    last = re.findall(r"0x[0-9a-fA-F]{8}u", source)[-1]
    text = "/* Temporary build-local private exact coefficients. Do not distribute. */\n"
    text += "#define MIAMI_N513_TAIL " + last + "\n"
    text += "static const uint8_t miami_n513_packed[] = {\n"
    text += "\n".join("    " + ", ".join(str(v) for v in data[i:i+16]) + ","
                     for i in range(0,len(data),16))
    text += "\n};\n"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text)
    print(f"Miami N513: 513 exact words, {len(data)} packed bytes")

if __name__ == "__main__":
    generate(*map(Path, sys.argv[1:]))
