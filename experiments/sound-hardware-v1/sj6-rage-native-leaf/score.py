"""Report strictly chronological gains and stereo influence, never fit alignment."""
import argparse, hashlib, json, math, struct
from pathlib import Path

def sha(data): return hashlib.sha256(data).hexdigest()
def gain_db(src, dst, lo, hi):
    a = math.fsum(v*v for v in src[lo:hi])
    b = math.fsum(v*v for v in dst[lo:hi])
    return 10*math.log10(b/a) if a and b else None

def load_planes(data, frames):
    if len(data) != frames*8:
        raise ValueError("wrong exact stereo frame count")
    values = struct.unpack("<%df"%(2*frames),data)
    if not all(map(math.isfinite,values)):
        raise ValueError("non-finite audio")
    return values[:frames],values[frames:]

def score(fixture, bypass, active, repeat):
    m = json.loads((fixture/'manifest.json').read_text())
    n,rate = m['frames'],m['rate']
    raw = (fixture/'input-planar.f32le').read_bytes()
    if sha(raw) != m['input_sha256']:
        raise ValueError("input differs from manifest")
    if raw != bypass.read_bytes():
        raise ValueError("declared wrapper dry reference differs from input")
    wet = active.read_bytes()
    if wet != repeat.read_bytes():
        raise ValueError("independent active outputs differ")
    x,y = load_planes(raw,n),load_planes(wet,n)
    spans={}
    for span in m['tracks']:
        name,lo,hi = (span[k] for k in ('name','from','to'))
        early = min(hi,lo+rate//100)
        late = max(lo,hi-rate//20)
        gains=lambda a,b:[gain_db(x[i],y[i],a,b) for i in range(2)]
        spans[name] = dict(first_10ms_gain_dB=gains(lo,early),last_50ms_gain_dB=gains(late,hi),
            output_peak=[max(abs(v) for v in channel[lo:hi]) for channel in y])
        if 'release' in name:
            step=rate//100
            spans[name]['first_100ms_gain_dB_per_10ms']=[gains(i,i+step) for i in range(lo,lo+step*10,step)]
    return dict(input_sha256=sha(raw),active_sha256=sha(wet),exact_dry=True,exact_repeat=True,
                aligned=False,metrics=spans)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for a in ('fixture','bypass','active','repeat'):
        p.add_argument(a,type=Path)
    print(json.dumps(score(**vars(p.parse_args())),indent=2))
