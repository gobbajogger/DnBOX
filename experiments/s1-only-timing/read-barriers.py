from pathlib import Path
import json, struct, subprocess, sys
keys='909 808 bass_a bass_b break body body_wait wait_909 wait_808 wait_a join post takeover_count'.split()
def load(d):
    d=Path(d)
    r=(d/'ram.bin').read_bytes()
    base=0x1c00000
    s={}
    for line in subprocess.check_output(['readelf','-Ws',d/'bench.elf'],text=True).splitlines():
        w=line.split()
        if len(w)>=8 and w[0][:-1].isdigit() and w[6]!='UND':
            s[w[7]]=int(w[1],16)-base
    assert 'skipped 0 cycles' in (d/'run.log').read_text()
    return r,s
if __name__=='__main__':
    kind=sys.argv[1]
    r,s=load(sys.argv[2])
    if kind=='audio':
        rows={}
        for i in range(10):
            frame, ticks, half,*c=struct.unpack_from('<29I',r,s['bench_barrier_rows']+i*29*4)
            if not ticks: continue
            rows[i]=dict(frame=frame,half=half,total_us=round(ticks/24,2),
                         cores=[{k:round(x/24,2) if k!='takeover_count' else x for k,x in zip(keys,c[j*13:]) if x} for j in range(2)])
        print(json.dumps(rows,indent=2))
    elif kind=='save':
        print(list(struct.unpack_from('<10I',r,s['bench_save'])))
        lo,hi=s['_ustack_lo'],s['_ustack_top']
        assert lo>0 and hi>lo
        depth=next((hi-i for i in range(lo,hi,4) if struct.unpack_from('<I',r,i)[0]!=0x5553544b),0)
        print(f'capacity={hi-lo} observed={depth} untouched={hi-lo-depth}')
