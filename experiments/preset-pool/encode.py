#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""DNL2 offline encoder. Input ordered engine-native (field ID, binary32 word) pairs.
The website owns persistent IDs; never regenerate them when filtering a bank.
This module does not read private presets, allocate device flash or emit MIDI.
"""
import collections
import struct
import zlib

HEADER = struct.Struct('<4sIIBBH8sHHHH7H')
MAX_FIELDS = (22, 18, 150, 220)


def encode(engine, bank_id, bank_name, rows):
    """rows: sequence of (8-byte preset ID, name, [(field ID, uint32 word), ...])."""
    if not 0 <= engine < len(MAX_FIELDS) or len(bank_id) != 8 or not 1 <= len(rows) <= 1024:
        raise ValueError('bank identity/count')
    bank_name = bank_name.encode('utf-8')
    if not bank_name or b'\0' in bank_name or len(bank_name) > 255:
        raise ValueError('bank name')
    seen = set()
    for pid, name, fields in rows:
        raw = name.encode('utf-8')
        if len(pid) != 8 or pid in seen or not 1 <= len(raw) <= 47 or b'\0' in raw:
            raise ValueError('preset identity/name')
        seen.add(pid)
        if not 1 <= len(fields) <= 220 or any(not 0 <= f < MAX_FIELDS[engine] or not 0 <= w <= 0xffffffff or (w & 0x7f800000) == 0x7f800000 for f, w in fields):
            raise ValueError('field ID/word/count')
    G = sorted({w for _, _, fields in rows for _, w in fields})
    if len(G) > 65535:
        raise ValueError('global dictionary count')
    gi = {w: i for i, w in enumerate(G)}
    gb = (len(G) - 1).bit_length()
    hist = collections.defaultdict(collections.Counter)
    ordered = []
    for _, _, fields in rows:
        r = [(f, gi[w]) for f, w in fields]
        ordered.append(r)
        for f, w in r:
            hist[f][w] += 1
    ids = sorted(hist)
    fi = {f: i for i, f in enumerate(ids)}
    palettes, dictionary, meta = [], bytearray(), bytearray()

    def index(xs):
        if any(not 0 <= x <= 65535 for x in xs):
            raise ValueError('offset overflow')
        return struct.pack('<' + 'H' * len(xs), *xs)

    for f in ids:
        freq = hist[f]
        values = sorted(freq, key=lambda w: (-freq[w], w))
        total = sum(freq.values())
        opts = []
        for k in range(9):
            n = min(len(values), 1 << k)
            n -= n < len(values)
            opts.append((16*n + total*k + (total-sum(freq[v] for v in values[:n]))*gb, k, n))
        _, k, n = min(opts)
        vals = values[:n]
        palettes.append((k, n, {w: i for i, w in enumerate(vals)}))
        meta += struct.pack('<BBHH', f, k, n, len(dictionary)//2)
        dictionary += index(vals)

    def bits(items):
        result = bytearray()
        acc = nb = 0
        for v, width in items:
            if not 0 <= v < 1 << width:
                raise ValueError('bit width')
            acc |= v << nb
            nb += width
            while nb >= 8:
                result.append(acc & 255)
                acc >>= 8
                nb -= 8
        if nb:
            result.append(acc)
        return result

    layouts, ld, li, rd, ri = {}, bytearray(), [], bytearray(), []
    for (pid, name, _), fields in zip(rows, ordered):
        layout = tuple(fi[f] for f, _ in fields)
        if layout not in layouts:
            if len(layouts) == 256:
                raise ValueError('too many layouts')
            layouts[layout] = len(layouts)
            li.append(len(ld))
            ld += bytes((len(layout), *layout))
        raw = name.encode('utf-8')
        ri.append(len(rd))
        rd += pid + bytes((layouts[layout], len(raw))) + raw
        codes = []
        for f, w in fields:
            k, n, table = palettes[fi[f]]
            v = table.get(w, n)
            codes.append((v, k))
            if v == n:
                codes.append((w, gb))
        rd += bits(codes)
    # 32 exact sorted words per block, uint16 block offset, width, base, deltas.
    offsets, gd = [], bytearray()
    for i in range(0, len(G), 32):
        group = G[i:i+32]
        ds = [b-a for a, b in zip(group, group[1:])]
        width = max(ds, default=0).bit_length()
        offsets.append(len(gd))
        gd += struct.pack('<BI', width, group[0]) + bits((d, width) for d in ds)
    parts = (bank_name, index(offsets)+gd, meta, dictionary, index(li)+ld, index(ri)+rd)
    off = [HEADER.size]
    for p in parts:
        off.append(off[-1]+len(p))
    if off[-1] > 65535:
        raise ValueError('bank exceeds format limit')
    header = HEADER.pack(b'DNL2', off[-1], 0, engine, 1, len(bank_name), bank_id, len(rows), len(G), len(ids), len(layouts), *off)
    blob = header + b''.join(parts)
    return blob[:8] + struct.pack('<I', zlib.crc32(blob[:8]+blob[12:])) + blob[12:]
