#!/usr/bin/env python3
"""Generated J60V2 coefficient candidates. No original assets or firmware."""
import argparse
import math
import struct

def f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]

def row(p, cutoff):
    h = []
    for j in range(-31, 33):
        t = p - j
        z = math.pi * cutoff * t
        g = math.sin(z) / z if z else 1.
        w = .42 + .5 * math.cos(math.pi*t/32) + .08 * math.cos(math.pi*t/16)
        h.append(f32(cutoff*g*w if abs(t) < 32 else 0.))
    norm = f32(sum(h))
    return tuple(f32(x/norm) for x in h)

def original_interpolated(p, cutoff):
    k = min(int(p*256), 255)
    u = f32(p*256-k)
    a, b = row(k/256, cutoff), row((k+1)/256, cutoff)
    return tuple(f32(x+u*f32(y-x)) for x, y in zip(a, b))

def spline(a, b, c, d, u):
    v1 = f32(c-a)
    v2 = f32(f32(f32(2*a-5*b)+4*c)-d)
    v3 = f32(f32(f32(-a+3*b)-3*c)+d)
    return f32(b+f32(.5*u)*f32(v1+u*f32(v2+u*v3)))

def knots(count, cutoff, quant=None):
    rows = [row(i/count, cutoff) for i in range(-1, count//2+3)]
    if quant:
        unit = 1 << quant
        rows = [tuple(round(v*unit)/unit for v in a) for a in rows]
    return rows

def spline_row(p, count, rows):
    flipped = p > .5
    if flipped:
        p = 1-p
    pos = p*count
    k = int(pos)
    result = tuple(spline(*v, f32(pos-k)) for v in zip(*rows[k:k+4]))
    return result[::-1] if flipped else result

def prove():
    for cutoff, den in ((1.,320), (147/320,147), (1.,2), (.5,1)):
        for count, quant in ((8,None), (16,15), (16,18)):
            table = knots(count, cutoff, quant)
            max_l1 = max(abs_sum(a-b for a,b in zip(original_interpolated(k/den, cutoff),
                         spline_row(k/den, count, table))) for k in range(den))
            print(f"cutoff={cutoff:.7f} phases={den} knots={count} q={quant} max_row_L1={max_l1:.8g}")
    assert row(0., 1.)[31] == 1.
    assert len(knots(8, .5)) == 8
    assert len(knots(16, .5)) == 12

def abs_sum(values):
    return sum(map(abs, values))

if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.parse_args()
    prove()
