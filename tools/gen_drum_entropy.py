#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Losslessly rebucket the three generated fixed IMA streams. No decoding during generation depends on target state."""
from collections import Counter, defaultdict
from heapq import heapify, heappop, heappush
from pathlib import Path
import re
import sys

FAST_BITS = 5
BOOKS = 12

def huffman(counts):
    nodes = [(count, (symbol,)) for symbol, count in counts.items()]
    heapify(nodes)
    depths = Counter()
    while len(nodes) > 1:
        n1, s1 = heappop(nodes)
        n2, s2 = heappop(nodes)
        symbols = s1 + s2
        depths.update(symbols)
        heappush(nodes, (n1+n2, symbols))
    code = old_width = 0
    result = {}
    for width, symbol in sorted((max(1, depths[s]), s) for s in counts):
        code <<= width-old_width
        result[symbol] = (code, width)
        code += 1
        old_width = width
    return result

def coded(counts):
    nodes = list(counts.values())
    if len(nodes) < 2:
        return sum(nodes)
    heapify(nodes)
    cost = 0
    while len(nodes) > 1:
        merged = heappop(nodes) + heappop(nodes)
        cost += merged
        heappush(nodes, merged)
    return cost

def books(samples):
    counts = [Counter() for _ in range(256)]
    for data in samples:
        ctx = 0
        for q in data:
            counts[ctx][q] += 1
            ctx = ((ctx << 4) | q) & 255
    groups = {i: c for i, c in enumerate(counts) if c}
    contexts = {i: [i] for i in groups}
    costs = {i: coded(c) for i, c in groups.items()}
    def edge(i, j):
        return coded(groups[i] + groups[j]) - costs[i] - costs[j], i, j
    edges = [edge(i, j) for i in groups for j in groups if i < j]
    heapify(edges)
    new_id = 256
    while len(groups) > BOOKS:
        while True:
            _, i, j = heappop(edges)
            if i in groups and j in groups:
                break
        groups[new_id] = groups.pop(i) + groups.pop(j)
        contexts[new_id] = contexts.pop(i) + contexts.pop(j)
        costs[new_id] = coded(groups[new_id])
        for other in groups:
            if other != new_id:
                heappush(edges, edge(other, new_id))
        new_id += 1
    lookup = [0] * 256
    chosen = []
    for key, group in groups.items():
        for ctx in contexts[key]:
            lookup[ctx] = len(chosen)
        chosen.append(huffman(group))
    return lookup, chosen

def array(kind, name, vals):
    yield f"static const {kind} {name}[{len(vals)}] = {{"
    for start in range(0, len(vals), 24):
        yield "    " + ", ".join(map(str, vals[start:start+24])) + ","
    yield "};"

def build(raw, output):
    text=raw.read_text()
    frames={name:int(n) for name,n in re.findall(r'#define (X0X_SMP_\w+)_LEN\s+(\d+)u',text)}
    samples={}
    for name,size,body in re.findall(r'(x0x_smp_\w+)\[(\d+)\] = \{(.*?)\};',text,re.S):
        data=bytes(int(n) for n in re.findall(r'\d+',body))
        want=frames[name.upper()]
        assert len(data)==int(size)==(want+1)//2
        samples[name]=[v for byte in data for v in (byte&15,byte>>4)][:want]
    assert len(samples)==3 and set(samples)=={"x0x_smp_hh","x0x_smp_ride","x0x_smp_crash"}
    ctxs, huff=books(samples.values())
    direct,extra,tail=[],[],[]
    for book in huff:
        row=[0]*(1<<FAST_BITS)
        over=defaultdict(list)
        for symbol,(code,width) in book.items():
            if width>FAST_BITS:
                over[code>>(width-FAST_BITS)].append((code,width,symbol))
                continue
            for j in range(1<<(FAST_BITS-width)):
                cell=(code<<(FAST_BITS-width))|j
                assert not row[cell]
                row[cell]=(width<<4)|symbol
        for prefix, codes in sorted(over.items()):
            w=max(width-FAST_BITS for _,width,_ in codes)
            assert not row[prefix] and len(extra)<128 and len(tail)<8192 and 0<w<8
            row[prefix]=128+len(extra)
            extra.append(len(tail)|(w<<13))
            extension=[0]*(1<<w)
            for code,width,symbol in codes:
                n=width-FAST_BITS
                suffix=code&((1<<n)-1)
                for j in range(1<<(w-n)):
                    cell=(suffix<<(w-n))|j
                    assert not extension[cell]
                    extension[cell]=(width<<4)|symbol
            tail.extend(extension)
        direct.extend(row)
    lines=["/* Generated from the exact same IMA source bytes. */","#pragma once","#include <stdint.h>",
           f"#define HD_PREFIX {FAST_BITS}u"]
    lines.extend(array("uint8_t","hd_ctx",[ctxs[i]|(ctxs[i+1]<<4) for i in range(0,256,2)]))
    lines.extend(array("uint8_t","hd_first",direct))
    lines.extend(array("uint16_t","hd_extra",extra))
    lines.extend(array("uint8_t","hd_tail",tail))
    payload=0
    for name,values in samples.items():
        ctx=code=n=0
        stream=[]
        for val in values:
            symbol,width=huff[ctxs[ctx]][val]
            ctx=((ctx<<4)|val)&255
            code=(code<<width)|symbol
            n+=width
            while n>=8:
                n-=8
                stream.append(code>>n)
                code&=(1<<n)-1
        if n:
            stream.append(code<<(8-n))
        assert len(stream)*8<0xffffff
        stream+= [0,0] # bounded final lookahead, never part of the IMA stream
        lines.extend(array("uint8_t",name,stream))
        lines.append(f"#define {name.upper()}_LEN {len(values)}u")
        payload+=len(stream)
    lines.append(r"""
/* state starts zero on trigger. The caller stops at the exact source length. */
static inline uint32_t d9_entropy_code(const uint8_t *data, uint32_t *state)
{
    unsigned bit=*state&0xffffffu, ctx=*state>>24, sh=bit&7u;
    const uint8_t *src=data+(bit>>3);
    unsigned x=(unsigned)src[0]<<8 | src[1];
    unsigned b=(hd_ctx[ctx>>1]>>((ctx&1u)*4u))&15u;
    unsigned e=hd_first[(b<<HD_PREFIX)|((x>>(16u-HD_PREFIX-sh))&((1u<<HD_PREFIX)-1u))];
    if (e&128u) {
        unsigned d=hd_extra[e&127u], w=d>>13;
        x=x<<8 | src[2];
        e=hd_tail[(d&8191u)+((x>>(24u-HD_PREFIX-w-sh))&((1u<<w)-1u))];
    }
    *state=(bit+(e>>4))|((ctx<<4|(e&15u))<<24);
    return e&15u;
}
""")
    tmp=output.with_suffix(".tmp")
    tmp.write_text("\n".join(lines)+"\n")
    tmp.replace(output)
    print(f"909 entropy: {sum(map(len,samples.values()))} samples, {payload} stream bytes + "
          f"{128+len(direct)+2*len(extra)+len(tail)} bytes of lookup")

if __name__=="__main__":
    build(Path(sys.argv[1]),Path(sys.argv[2]))
