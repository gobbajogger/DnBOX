from pathlib import Path
from textwrap import dedent
def edit(f,old,new):
 p=Path('tree')/f
 s=p.read_text()
 assert s.count(old)==1,(f,old,s.count(old))
 p.write_text(s.replace(old,new))
src='tools/gen_drum_samples.py'
edit(src,'import hashlib\n','import hashlib\nimport heapq\n')
tag='def main():'
fn='''def huffman_cymbals(samples):
    """Deterministic shared 6-bit-prefix coding of the exact IMA nibbles."""
    nibbles = [[(data[i >> 1] >> (4 * (i & 1))) & 15 for i in range(n)]
               for _, n, data in samples]
    counts = [0] * 16
    for voice in nibbles:
        for x in voice:
            counts[x] += 1
    heap = [(n, (x,)) for x, n in enumerate(counts)]
    heapq.heapify(heap)
    lens = [0] * 16
    while len(heap) > 1:
        a, kids_a = heapq.heappop(heap)
        b, kids_b = heapq.heappop(heap)
        kids = kids_a + kids_b
        for x in kids:
            lens[x] += 1
        heapq.heappush(heap, (a + b, kids))
    assert max(lens) == 6
    lookup = [0] * 64
    codes = [0] * 16
    code = previous = 0
    for x in sorted(range(16), key=lambda x: (lens[x], x)):
        n = lens[x]
        code <<= n - previous
        bit_order = sum(((code >> k) & 1) << (n - 1 - k) for k in range(n))
        codes[x] = bit_order
        for entry in range(bit_order, 64, 1 << n):
            assert lookup[entry] == 0
            lookup[entry] = (n << 4) | x
        code += 1
        previous = n
    assert all(lookup)
    output = []
    for (name, _, _), voice in zip(samples, nibbles):
        raw = bytearray()
        word = bits = 0
        for x in voice:
            word |= codes[x] << bits
            bits += lens[x]
            if bits >= 8:
                raw.append(word & 255)
                word >>= 8
                bits -= 8
        if bits:
            raw.append(word)
        raw.append(0)  # final fixed 2-byte lookahead may reach this guard byte
        output.append((name, raw))
    return lookup, output


'''
edit(src,tag,fn+tag)
edit(src,'    for name, fn in SAMPLES:\n','    streams, plain = [], []\n    for name, fn in SAMPLES:\n')
edit(src,'''        hdr.append(arr("uint8_t", name, list(data), per=24))
        hdr.append("")
    write_header(out_s, "\\n".join(hdr))''','''        streams.append((name, len(pcm), data))
        plain.append(arr("uint8_t", name, list(data), per=24))
    lut, packed = huffman_cymbals(streams)
    hdr += ["", "#if X0X_909_HUFF_DIAG", arr("uint8_t", "x0x_smp_huff_lut", lut, per=16)]
    hdr += [arr("uint8_t", name, list(data), per=24) for name,data in packed]
    hdr += ["#else", *plain, "#endif", ""]
    write_header(out_s, "\\n".join(hdr))''')
i='firmware/src/dsp/ima_adpcm.h'
old='''static inline int32_t ima_decode(const uint8_t *d, uint32_t i, int32_t *pred, int32_t *idx)
{
    uint32_t byte = d[i >> 1], code = (i & 1u) ? (byte >> 4) : (byte & 15u);'''
edit(i,old,'''static inline int32_t ima_decode_code(uint32_t code, int32_t *pred, int32_t *idx)
{''')
p=Path('tree')/i
p.write_text(p.read_text()+'''static inline int32_t ima_decode(const uint8_t *d, uint32_t i, int32_t *pred, int32_t *idx)
{
    uint32_t byte = d[i >> 1], code = (i & 1u) ? (byte >> 4) : (byte & 15u);
    return ima_decode_code(code, pred, idx);
}
''')
edit('firmware/src/dsp/drum909.h','    int32_t pred, idx;            /* decoder state after dk samples */', '''#if X0X_909_HUFF_DIAG
    uint32_t huff_bit;            /* next exact sequential IMA nibble's bit cursor */
#endif
    int32_t pred, idx;            /* decoder state after dk samples */''')
d='firmware/src/dsp/drum909.c'
edit(d,'        s->pred = s->idx = 0;', '''#if X0X_909_HUFF_DIAG
        s->huff_bit = 0;
#endif
        s->pred = s->idx = 0;''')
marker='/* Hats, ride, crash (render_sampler) */'
edit(d,marker,'''#if X0X_909_HUFF_DIAG
#if !X0X_COLD_COMPANION_LAB
#error "Huffman 909 diagnostic requires the verified read-only companion"
#endif
static inline int32_t d9_smp_decode(d9_smp_t *s, const uint8_t *stream)
{
    const uint32_t cursor = s->huff_bit;
    const uint8_t *p = stream + (cursor >> 3);
    const unsigned bits = p[0] | ((unsigned)p[1] << 8);
    const unsigned e = x0x_smp_huff_lut[(bits >> (cursor & 7u)) & 63u];
    s->huff_bit = cursor + (e >> 4);
    ++s->dk;
    return ima_decode_code(e & 15u, &s->pred, &s->idx);
}
#else
static inline int32_t d9_smp_decode(d9_smp_t *s, const uint8_t *stream)
{
    return ima_decode(stream, s->dk++, &s->pred, &s->idx);
}
#endif
''' + marker)
p=Path('tree')/d;s=p.read_text();old='ima_decode(adpcm, s->dk++, &s->pred, &s->idx)'
assert s.count(old)==3
p.write_text(s.replace(old,'d9_smp_decode(s, adpcm)'))
