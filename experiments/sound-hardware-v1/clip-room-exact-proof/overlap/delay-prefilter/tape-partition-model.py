"""One f32 falling-time counterexample for the historical deferred FX tile."""
from struct import pack, unpack

def f(x):
    return unpack("<f", pack("<f", x))[0]

def mono_h16_partition(dt, sr=44100, pool=2300):
    t = f(dt * sr)
    for dec in range(1,4096):
        d=f(t/dec)
        la=(int(f(2*d))+18)&~15
        lc=(int(d)+18)&~15
        if 2*(la+lc) <= pool:
            return dec, la, lc
    raise AssertionError

rate=f(.99)
complement=f(1-rate)
target=f(.2)
cur=f(.3)
rows=[]
for _ in range(32):
    cur = f(f(complement*target) + f(rate*cur))
    rows.append(cur)

first=mono_h16_partition(max(target,rows[0]))
last=mono_h16_partition(max(target,rows[-1]))
assert first == (36,736,384) and last == (33,736,368)
print(rows[0],first,rows[-1],last)
