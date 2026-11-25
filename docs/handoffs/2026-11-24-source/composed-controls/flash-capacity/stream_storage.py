"""Offline byte-exact compression measurements for linked FDATA ADPCM assets."""
import collections
import heapq
import json
import lzma
import re
import struct
import sys
from pathlib import Path

SECTOR = 4096
ASSET = r'x0x_smp_(?:crash|ride|hh)'

def model(count, alphabet):
    heap = [(n, i, [i]) for i, n in count.items()]
    heapq.heapify(heap)
    uid = alphabet
    lengths = [0] * alphabet
    while len(heap) > 1:
        a, _, low = heapq.heappop(heap)
        b, _, high = heapq.heappop(heap)
        leaves = low + high
        for i in leaves:
            lengths[i] += 1
        heapq.heappush(heap, (a + b, uid, leaves))
        uid += 1
    if len(heap) == 1 and len(heap[0][2]) == 1:
        lengths[heap[0][2][0]] = 1
    return lengths

def canonical(lengths):
    code = 0
    prior = 0
    enc = {}
    dec = {}
    for k, l in sorted(enumerate(lengths), key=lambda x: (x[1], x[0])):
        if not l:
            continue
        code <<= l-prior
        enc[k] = (code, l)
        dec[l, code] = k
        prior = l
        code += 1
    return enc, dec

def symbols(block, alphabet):
    if alphabet == 256:
        return block
    return [y for x in block for y in (x >> 4, x & 15)]

def huff_asset(asset, block_size, alphabet, predictive):
    blocks = [asset[p:p+block_size] for p in range(0, len(asset), block_size)]
    counts = collections.defaultdict(collections.Counter)
    for block in blocks:
        c = 0
        for s in symbols(block, alphabet):
            counts[c][s] += 1
            c = s if predictive else 0
    nk = alphabet if predictive else 1
    # One byte per length: explicit model size, rather than assuming free tables.
    lens = b''.join(bytes(model(counts[k], alphabet)) for k in range(nk))
    trees = [canonical(lens[k:k+alphabet]) for k in range(0, len(lens), alphabet)]
    bodies = []
    for block in blocks:
        c = 0
        bit = word = 0
        encoded = bytearray()
        for s in symbols(block, alphabet):
            v, n = trees[c][0][s]
            c = s if predictive else 0
            word = (word << n) | v
            bit += n
            while bit >= 8:
                bit -= 8
                encoded.append(word >> bit)
                word &= (1 << bit) - 1
        if bit:
            encoded.append(word << (8-bit))
        bodies.append(bytes(encoded) if len(encoded)<len(block) else block)
    # 15-bit starts + raw flag in high bit; asset header gives source length.
    offset = 0
    directory = bytearray()
    for orig, enc in zip(blocks, bodies):
        assert offset < 0x8000
        directory += struct.pack('<H', offset | (0x8000 if len(enc) == len(orig) else 0))
        offset += len(enc)
    payload = struct.pack('<IHBB', len(asset), block_size, alphabet == 256, predictive)
    payload += lens + directory + b''.join(bodies)
    size, step, is_byte, pred = struct.unpack_from('<IHBB', payload)
    al = 256 if is_byte else 16
    model_size = al * (al if pred else 1)
    decs = [canonical(payload[i:i+al])[1] for i in range(8, 8+model_size, al)]
    count = (size+step-1)//step
    start = 8 + model_size + 2*count
    offsets = struct.unpack_from('<%dH'%count, payload, 8+model_size)
    decoded = []
    for q, loc in enumerate(offsets):
        off = start + (loc & 0x7fff)
        limit = min(size-q*step,step)
        if loc & 0x8000:
            decoded.append(payload[off:off+limit])
            continue
        c = 0
        values = []
        word = bit = 0
        end = (start + (offsets[q+1] & 0x7fff)) if q+1 < count else len(payload)
        for byte in payload[off:end]:
            for pos in range(7,-1,-1):
                word = (word << 1) | ((byte>>pos)&1)
                bit += 1
                sym = decs[c].get((bit,word))
                if sym is not None:
                    values.append(sym)
                    word = bit = 0
                    c = sym if pred else 0
            if len(values) >= limit * (2 if al==16 else 1):
                break
        values = values[:limit * (2 if al==16 else 1)]
        decoded.append(bytes(values) if al==256 else bytes((x<<4)|y for x,y in zip(values[::2], values[1::2])))
    assert b''.join(decoded) == asset
    return len(payload)

def block_lzma(asset, step):
    filters = [dict(id=lzma.FILTER_LZMA2, dict_size=4096)]
    chunks = [asset[p:p+step] for p in range(0,len(asset),step)]
    total = 8 + 2*len(chunks)
    for b in chunks:
        comp = lzma.compress(b,format=lzma.FORMAT_RAW,filters=filters)
        if len(comp) < len(b):
            assert lzma.decompress(comp,format=lzma.FORMAT_RAW,filters=filters)==b
        total += min(len(b),len(comp))
    return total

def resident(size):
    return ((size + SECTOR - 1) // SECTOR) * SECTOR

def break_slots(path):
    import hashlib
    meta=json.loads((path/'default-breaks.json').read_text())['assets']
    groups={1:[],2:[]}
    for item in meta:
        b=(path/(item['id']+'.adpcm')).read_bytes()
        assert len(b)==item['adpcm_bytes']
        assert hashlib.sha256(b).hexdigest()==item['adpcm_sha256']
        groups[item['slot']].append(b)
    for slot,clips in groups.items():
        fixed=512
        original=fixed+sum(map(len,clips))
        for step in (128,256,512,1024,2048,4096):
            # Decoder ID in the asset header selects the common canonical
            # Huffman representation with existing raw blocks as fallback.
            sizes=[min(huff_asset(b,step,16,False),huff_asset(b,step,256,False)) for b in clips]
            total=fixed+sum(sizes)
            print(json.dumps(dict(break_slot=slot,restart_adpcm_bytes=step,original=original,
                  encoded_B=total,aligned_B=resident(total),freed_sectors=(resident(original)-resident(total))//SECTOR)))

if __name__ == '__main__':
    if len(sys.argv)==3 and sys.argv[1]=='--default-breaks':
        break_slots(Path(sys.argv[2]))
        sys.exit(0)
    path=Path(sys.argv[1])
    bits=(path/'lab_fdata.bin').read_bytes()
    mapping=(path/'lab.map').read_text().split('.lab_slot3',1)[0]
    base=int(re.search(r'^\.lab_fdata\s+(0x[\da-f]+)',mapping,re.M)[1],16)
    ranges=[(asset,int(o,16)-base,int(n,16)) for asset,o,n in re.findall(r'^ \.rodata\.('+ASSET+r')\s+(0x[\da-f]+)\s+(0x[\da-f]+)',mapping,re.M)]
    assert len(ranges)==3
    assets={n:bits[p:p+k] for n,p,k in ranges}
    fixed=len(bits)-sum(len(b)for b in assets.values())
    print(json.dumps(dict(original_fdata=len(bits),fixed=fixed,adpcm={n:len(b)for n,b in assets.items()})))
    for kind in ('nibble','prior-nibble','byte','lzma-4k'):
        for step in (128,256,512,1024,2048,4096):
            sizes={}
            for n,b in assets.items():
                sizes[n]=block_lzma(b,step) if kind=='lzma-4k' else huff_asset(b,step,256 if kind=='byte' else 16,kind=='prior-nibble')
            total=fixed+sum(sizes.values())
            print(json.dumps(dict(kind=kind,restart_adpcm_bytes=step,asset_stored_B=sizes,FDATA_B=total,FDATA_sector_B=resident(total),freed_sectors=(resident(len(bits))-resident(total))//SECTOR)))
