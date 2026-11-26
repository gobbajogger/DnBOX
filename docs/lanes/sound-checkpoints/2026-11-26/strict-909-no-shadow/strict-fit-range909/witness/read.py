"""Validate a one-record attribute-only firmware capture, never a release verdict."""
import sys, struct, json
from pathlib import Path
p=Path(__file__).resolve().parent
sys.path.insert(0,str(p.parent.parent/'restored-harness-inspect/helpers'))
import guarded_replay as g
def row(payload, count, late, max_us, renders):
    assert count==late and (len(payload),) == (20,)
    if count==0:return {'observed_no_late':True,'coverage':'entire instrumented shot, not release'}
    a,t,t5,mask,b,c,n,flags=struct.unpack('<IIIIBBBB',payload)
    assert 0<=a<renders and 0<t<=max_us and 0<=t5<=t*24+23
    assert 0<=b<2 and 0<=c<2 and b!=c and n>0
    return dict(worst_late_ordinal=a, elapsed_us=t,nested_timer5_ticks=t5,
      plans=[j for j in range(32) if mask&(1<<j)],before_dma=b,after_dma=c,spans=n,
      had_808_prefix=bool(flags&1),launch_fallback=bool(flags&2),open_join=bool(flags&4),late_count=late)
if __name__=='__main__':
    elf,pin,ram=sys.argv[1:]
    blob,sym,sec=g.symbols_and_sections(Path(elf)); assert g.sha(blob)==pin
    d=Path(ram).read_bytes()
    def at(n,k):
        a=sym[n]-g.ELF_RAM
        assert 0<=a<=len(d)-k
        return d[a:a+k]
    for name in ('ram_text','c1_text'):
        a,n,data=sec['.'+name]; i=a-g.ELF_RAM
        assert n==len(data) and data==d[i:i+n]
    dbg=struct.unpack('<13I',at('x0x_dbg',52)); assert dbg[0]==0x44424731
    count,=struct.unpack('<I',at('lab_late_count',4))
    raw=at('lab_late_worst',20)
    result=row(raw,count,dbg[5],dbg[2],dbg[1])
    result.update(elf=pin,ram_sha=g.sha(d),comparison='instrumented scope only; no external event anchor')
    print(json.dumps(result,indent=2))
