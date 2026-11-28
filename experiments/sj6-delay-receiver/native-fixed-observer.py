"""Read completed phase observations from fixed-sample emulator fixtures."""
import json
from pathlib import Path
import struct
import subprocess
import sys

def decode(path):
    d = Path(path)
    table = {}
    for line in subprocess.check_output(['readelf', '-Ws', d/'bench.elf'], text=True).splitlines():
        t = line.split()
        if len(t) >= 8 and t[3] == 'OBJECT':
            table[t[7]] = int(t[1], 16)-0x01c00000, int(t[2],0)
    ram = (d/'ram.bin').read_bytes()
    def get(name, shape):
        offset, size = table[name]
        assert size == struct.calcsize('<'+shape), (name, size, shape)
        return struct.unpack_from('<'+shape, ram, offset)
    assert get('bench_fixed', '10I')[:8] == (0,10,3679125248,65536,98304,163840,4,0)
    assert "skipped 0 cycles" in (d/'run.log').read_text()
    return get, table, ram

def timing(path):
    get,table,ram=decode(path)
    assert get('bench_phase_rejected','I') == (0,)
    off,size=table['bench_half_flags']
    flags=ram[off:off+size]
    stats=get('bench_phase','6I')
    dbg=get('x0x_dbg','13I')
    length=sum(stats[::2])
    assert len(flags)>=length and len(flags)>=320
    assert all(x & 8 and x < 16 for x in flags[:length]) and not any(flags[length:])
    assert length==dbg[1]
    assert max(stats[1::2])//24==dbg[2]
    assert length>=320 and stats[2]==128
    assert sum(bool(r&4) for r in flags)==dbg[5]==get('audio_xruns','I')[0]
    phases={}
    for p,(label,lo,hi) in enumerate([('before',0,192),('held',192,320),('release',320,length)]):
        assert hi-lo==stats[2*p]
        row=flags[lo:hi]
        phases[label]=dict(halves=len(row), peak_ticks=stats[2*p+1], peak_us=stats[2*p+1]/24,
          over_target=[i+lo for i,v in enumerate(row) if v&1],
          over_physical_half=[i+lo for i,v in enumerate(row) if v&2],
          dma_index_flips=[i+lo for i,v in enumerate(row) if v&4])
    return phases

def pcm(path):
    get,table,ram=decode(path)
    assert get('bench_pcm_count','I')==(160,) and get('bench_pcm_rejected','I')==(0,)
    off,n=table['bench_pcm_sig']
    assert n==160*4
    return tuple(struct.iter_unpack('<I',ram[off:off+n]))

if __name__=='__main__':
    mode, before,after=sys.argv[1:]
    if mode=='timing':
        a,b=timing(before),timing(after)
        print(json.dumps({'base':a,'candidate':b},indent=2))
    elif mode=='pcm':
        a,b=pcm(before),pcm(after)
        delta=[i+192 for i,(x,y) in enumerate(zip(a,b)) if x!=y]
        print(json.dumps({'generated_halves':160,'different_digest_halves':len(delta),'indices':delta}))
    else:
        raise SystemExit('mode: timing or pcm')
