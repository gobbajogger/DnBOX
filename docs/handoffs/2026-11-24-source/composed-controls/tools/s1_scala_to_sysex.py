#!/usr/bin/env python3
"""Encode an octave-based Scala scale for DnBOX's Synth One USER scale.
This writes a local SysEx frame only. It does not connect to MIDI or save a project.
"""
from argparse import ArgumentParser
from fractions import Fraction
from math import exp2, frexp, isfinite
from pathlib import Path
from struct import pack, unpack
from zlib import crc32

def read(path):
    lines = [line.split('!',1)[0].strip() for line in path.read_text(encoding='utf-8-sig').splitlines()]
    lines = [line for line in lines if line]
    if len(lines)<3:
        raise ValueError('missing description, count, or scale entries')
    n=int(lines[1])
    if not 1<=n<=128 or len(lines)!=2+n:
        raise ValueError('expected exactly 1..128 Scala degrees')
    pitches=[1.0]  # Scala omits the 1/1 degree; final entry is the period
    for line in lines[2:]:
        token=line.split()[0]
        if len(token)>64:
            raise ValueError('Scala number too long')
        ratio=exp2(float(token)/1200.0) if '.' in token else float(Fraction(token))
        if not isfinite(ratio) or ratio<=0:
            raise ValueError('nonpositive or nonfinite ratio')
        pitches.append(ratio)
    if abs(pitches[-1]-2.0)>1e-9:
        raise ValueError('Synth One User scales require an octave period (2/1 or 1200.0 cents)')
    ratios=sorted(frexp(r)[0]*2 for r in pitches[:-1])  # normalize/sort as AKTuningTable
    name=lines[0].encode('ascii','replace')[:12]
    name=bytes(c if 32<=c<=126 else 32 for c in name).ljust(12,b'\0')
    # Each binary32 ratio lies in [1,2); exponent/sign are implied. A
    # ratio just below 2 can round to 2.0 at binary32 precision. Keep its
    # scale ordering by saturating to the closest representable in-domain value.
    codes=[min(0x7fffff,unpack('<I',pack('<f',r))[0]-0x3f800000) for r in ratios]
    return n,name+b''.join(code.to_bytes(3,'little') for code in codes)

def pack7(raw):
    for start in range(0,len(raw),7):
        group=raw[start:start+7]
        yield sum((c>>7)<<i for i,c in enumerate(group))
        yield from (c&127 for c in group)

def main():
    p=ArgumentParser(description=__doc__)
    p.add_argument('scala',type=Path)
    p.add_argument('sysex',type=Path)
    p.add_argument('--owner',type=str.upper,choices=('A','B'),required=True)
    a=p.parse_args()
    try:
        n,raw=read(a.scala)
    except (OSError,ValueError,OverflowError,ZeroDivisionError) as exc:
        p.error(str(exc))
    check=crc32(raw)
    frame=bytes([0xf0,0x7d,0x46,0x4c,53,1,int(a.owner=='B'),n-1,
                 *((check>>(7*i))&127 for i in range(5)),*pack7(raw),0xf7])
    a.sysex.write_bytes(frame)

if __name__=='__main__':
    main()
