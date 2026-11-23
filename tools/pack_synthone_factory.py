#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Pack the 694 Synth One factory programs into one position-independent blob.

Input: the 12 authenticated bank JSONs (assets/factory_banks/synthone) compiled
to ordered physical writes by tools/compile-synthone-factory.py.  Output: a
little-endian blob read in place by firmware/src/dsp/s1pack/s1pack.c from any
4-byte-aligned base (XIP window or RAM).  It holds no absolute addresses.

Layout (all bit fields MSB first):
  header 32 B: "S1PK" u16 version=1 u16 programs u16 columns u8 banks u8 rsv
               u32 total bytes  u32 crc32 (zlib, of bytes 32..total)
               u32 off_banks u32 off_progs u32 off_cols
  banks:  per bank u16 first program, u16 count, u8 first position,
          NUL-terminated name
  progs:  per program u24 bit offset (into the column/program stream area)
  stream: name-Huffman table, then per column (id, exp-Golomb k, canonical
          Huffman counts, dictionary in canonical order as zigzag
          exp-Golomb deltas of order-preserving float keys), then per
          program: Huffman name bytes + NUL, then one Huffman symbol per column.
"""
import argparse, hashlib, heapq, json, struct, subprocess, sys, tempfile, zlib
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BANKS = ROOT / 'assets/factory_banks/synthone'
MAXLEN = 15
MAGIC = b'S1PK'
VERSION = 1
HEADER = 32


class Bits:
    def __init__(self):
        self.v = bytearray(); self.n = 0
    def put(self, value, width):
        for i in range(width - 1, -1, -1):
            if self.n % 8 == 0: self.v.append(0)
            if (value >> i) & 1: self.v[-1] |= 0x80 >> (self.n % 8)
            self.n += 1
    def egk(self, value, k):          # exp-Golomb order k, value >= 0
        value += 1 << k
        width = value.bit_length()
        self.put(0, width - 1 - k); self.put(value, width)


def key(f32):                          # order-preserving uint32 for binary32 bits
    b = struct.unpack('<I', struct.pack('<f', f32))[0]
    return b ^ 0xFFFFFFFF if b & 0x80000000 else b | 0x80000000


def zig(d):
    return (d << 1) if d >= 0 else ((-d << 1) - 1)


def huff_lengths(freq):
    """Length-limited (MAXLEN) Huffman code lengths for {symbol: count}."""
    if len(freq) == 1:
        return {next(iter(freq)): 1}
    scale = 0
    while True:
        heap = [(max(1, c >> scale), i, (s,)) for i, (s, c) in enumerate(sorted(freq.items()))]
        heapq.heapify(heap)
        depth = Counter(); tie = len(heap)
        while len(heap) > 1:
            a = heapq.heappop(heap); b = heapq.heappop(heap)
            for s in a[2] + b[2]: depth[s] += 1
            heapq.heappush(heap, (a[0] + b[0], tie, a[2] + b[2])); tie += 1
        if max(depth.values()) <= MAXLEN:
            return dict(depth)
        scale += 1


def canonical(lengths, order_key):
    """Symbols in canonical order and their codes."""
    syms = sorted(lengths, key=lambda s: (lengths[s], order_key(s)))
    codes, code, prev = {}, 0, 0
    for s in syms:
        code <<= lengths[s] - prev; prev = lengths[s]
        codes[s] = code; code += 1
    counts = [sum(1 for s in syms if lengths[s] == L) for L in range(1, max(lengths.values()) + 1)]
    return syms, codes, counts


def best_k(values):
    def cost(k): return sum(2 * ((v + (1 << k)).bit_length()) - 1 - k for v in values)
    return min(range(31), key=cost)


def load_programs(commands_json):
    cases = json.loads(Path(commands_json).read_bytes())['cases']
    by_bank = {}
    for c in cases:
        by_bank.setdefault(c['bank'], {})[c['preset_index']] = c
    programs, banks = [], []
    for entry in sorted(json.loads((BANKS / 'MANIFEST.json').read_bytes())['files'], key=lambda b: b['name']):
        raw = (BANKS / entry['name']).read_bytes()
        if hashlib.sha256(raw).hexdigest() != entry['sha256']:
            raise SystemExit(f'pack: changed bank {entry["name"]}')
        rows = sorted(enumerate(json.loads(raw)), key=lambda r: r[1]['position'])
        first = rows[0][1]['position']
        banks.append((entry['name'][:-5], len(programs), len(rows), first))
        for off, (index, preset) in enumerate(rows):
            case = by_bank[entry['name']][index]
            if preset['position'] != first + off or preset['uid'] != case['uid'] or preset['name'] != case['name']:
                raise SystemExit(f'pack: binding mismatch {entry["name"]} {index}')
            programs.append((preset['name'], case['commands']))
    order = [i for i, _ in programs[0][1]]
    if len(programs) != 694 or any([i for i, _ in p[1]] != order for p in programs):
        raise SystemExit('pack: unexpected corpus')
    return programs, banks, order


GSTEP = 256   # global dictionary checkpoint interval
PSTEP = 8     # program offset interval; names restart front coding here
ESC = -1      # column symbol: value read as a global index


def put_table(s, lens, order_key, width):
    """Canonical Huffman table: L(4), counts eg0, symbols(width) in canonical order."""
    syms, codes, counts = canonical(lens, order_key)
    s.put(len(counts), 4)
    for c in counts: s.egk(c, 0)
    for x in syms: s.put(x, width)
    return codes


def pack(programs, banks, order):
    ncol = len(order)
    cols = [[key(p[1][c][1]) for p in programs] for c in range(ncol)]
    names = [p[0].encode('utf-8') for p in programs]
    if any(0 in n or len(n) > 47 for n in names):
        raise SystemExit('pack: bad name')
    G = sorted(set(v for col in cols for v in col))
    gi = {v: i for i, v in enumerate(G)}
    gbits = (len(G) - 1).bit_length()
    # names: front coding against the previous name (reset at each PSTEP checkpoint)
    prefix, rest = [], []
    for p, n in enumerate(names):
        prev = names[p - 1] if p % PSTEP else b''
        k = 0
        while k < min(len(prev), len(n), 31) and prev[k] == n[k]: k += 1
        prefix.append(k); rest.append(n[k:] + b'\0')
    s = Bits(); marks = {}
    plen = huff_lengths(Counter(prefix))
    pcodes = put_table(s, plen, lambda x: x, 5)
    clen = huff_lengths(Counter(b for r in rest for b in r))
    ccodes = put_table(s, clen, lambda x: x, 8)
    # per column: id(8), escape position eg0 (0 = none), L(4), counts eg0
    marks['columns'] = s.n
    colsyms = []
    for c in range(ncol):
        f = Counter(cols[c])
        esc = {v for v, n in f.items() if n == 1}
        if len(esc) < 2: esc = set()
        syms = [ESC if v in esc else gi[v] for v in cols[c]]
        lens = huff_lengths(Counter(syms))
        order_syms, codes, counts = canonical(lens, lambda x: x)
        s.put(order[c], 8)
        s.egk(order_syms.index(ESC) + 1 if esc else 0, 0)
        s.put(len(counts), 4)
        for n in counts: s.egk(n, 0)
        colsyms.append((syms, [x for x in order_syms if x != ESC], codes, lens))
    # per column dictionary: k(4), zigzag eg(k) deltas of global indices, canonical order
    marks['dictionaries'] = s.n
    for _, entries, _, _ in colsyms:
        prev, d = 0, []
        for g in entries:
            d.append(zig(g - prev)); prev = g
        k = best_k(d) if d else 0
        if k > 15: raise SystemExit('pack: column k')
        s.put(k, 4)
        for x in d: s.egk(x, k)
    # global values: kG(5); every GSTEP entries a raw 32-bit key, else eg(kG) delta
    marks['global'] = s.n
    deltas = [G[i] - G[i - 1] for i in range(1, len(G)) if i % GSTEP]
    kg = best_k(deltas)
    s.put(kg, 5)
    gchk = []
    for i, v in enumerate(G):
        if i % GSTEP == 0:
            gchk.append(s.n); s.put(v, 32)
        else:
            s.egk(v - G[i - 1], kg)
    # programs: prefix code, name tail up to NUL, then one symbol per column
    marks['programs'] = s.n
    pchk = []
    for p in range(len(programs)):
        if p % PSTEP == 0: pchk.append(s.n)
        s.put(pcodes[prefix[p]], plen[prefix[p]])
        for b in rest[p]: s.put(ccodes[b], clen[b])
        for c in range(ncol):
            syms, _, codes, lens = colsyms[c]
            x = syms[p]; s.put(codes[x], lens[x])
            if x == ESC: s.put(gi[cols[c][p]], gbits)
    marks['end'] = s.n
    if s.n >= 1 << 24:
        raise SystemExit('pack: stream too long')
    u24 = lambda xs: b''.join(x.to_bytes(3, 'little') for x in xs)
    bank_bytes = b''.join(struct.pack('<HHB', f, n, pos) + name.encode() + b'\0' for name, f, n, pos in banks)
    tables = u24([marks['columns'], marks['dictionaries'], marks['global']]) + u24(gchk) + u24(pchk)
    off_banks = HEADER
    off_tables = off_banks + len(bank_bytes)
    off_stream = (off_tables + len(tables) + 3) & ~3
    body = bank_bytes + tables + bytes(off_stream - off_tables - len(tables)) + bytes(s.v) + bytes(4)
    total = HEADER + len(body)
    body += bytes((-total) & 3); total = HEADER + len(body)
    head = MAGIC + struct.pack('<HHHBBII', VERSION, len(programs), ncol, len(banks), gbits, total, zlib.crc32(body))
    head += struct.pack('<HHHHHH', off_banks, off_tables, off_stream, len(G), GSTEP, PSTEP)
    assert len(head) == HEADER
    blob = head + body
    sizes = {
        'header': HEADER, 'bank_table': len(bank_bytes), 'index_tables': off_stream - off_tables,
        'name_code_tables': marks['columns'] // 8,
        'column_codes': (marks['dictionaries'] - marks['columns']) // 8,
        'column_dictionaries': (marks['global'] - marks['dictionaries']) // 8,
        'global_values': (marks['programs'] - marks['global']) // 8,
        'program_records': (marks['end'] - marks['programs']) // 8,
        'total': len(blob), 'global_entries': len(G),
        'column_entries': sum(len(c[1]) for c in colsyms)}
    return blob, sizes


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('out', type=Path)
    ap.add_argument('--commands', type=Path, help='ORIGINAL-PRESET-COMMANDS.json (default: compile now)')
    a = ap.parse_args()
    if a.commands is None:
        tmp = Path(tempfile.mkdtemp())
        subprocess.run([sys.executable, str(ROOT / 'tools/compile-synthone-factory.py'), '--out', str(tmp)],
                       check=True, stdout=subprocess.DEVNULL)
        a.commands = tmp / 'ORIGINAL-PRESET-COMMANDS.json'
    blob, sizes = pack(*load_programs(a.commands))
    a.out.write_bytes(blob)
    print(json.dumps(sizes))


if __name__ == '__main__':
    main()
