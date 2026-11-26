"""Event-only source-anchor reconstruction. Compare with fresh marker guest.
No sampling, PCM output, or guessed timer."""
from pathlib import Path
import ctypes, math, struct, json
f32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
lib=ctypes.CDLL('libm.so.6')
fma=lib.fma
fma.argtypes=[ctypes.c_double]*3
fma.restype=ctypes.c_double
def rebuild(p, held, released):
    # Stage entry descriptors and elapsed source preps only.
    # Loop at most one source recurrence per elapsed prep, on event path only.
    r=f32(44100/16)
    bendA=float.fromhex('0x1.c8f87724b5c1dp-3')
    bendD=float.fromhex('0x1.d037e1b1c8ff7p-8')
    ts=[int(f32(p[i]*r)) for i in (10,11,12,14,15)]
    a,h,d,sh,rl=ts
    s=p[13]
    calls=0
    def run(x,end,bend,ticks,count):
        nonlocal calls
        if x==end and ticks<=0:return end,count
        if x==end:return x,max(0,count-ticks)
        m=math.exp(-math.log((bend+abs(end-x))/bend)/ticks)
        up=end>x
        add=(end+(bend if up else -bend))*(1-m)
        for i in range(count):
            calls+=1
            x=fma(m,x,add)
            if x>=end if up else x<=end:
                return end,count-1-i
        return x,0
    # Source enters a zero-duration HOLD from Attack completion; one prep
    # consumes it before Decay, but release entry skips authored zero Hold.
    y,n=run(0.,1.,bendA,a,held)
    if n:
        n=max(0,n-max(1,h))
        y,_=run(1.,s,bendD,d,n)
    if released is not None:
        # Current receiver preserves live binary64 anchor on zero-hold release.
        y,_=run(y,0.,bendD,rl,max(0,released-sh))
    return y,calls
def main():
    x=Path('recovered-sound/sj6-sub-handoff/shared-clock/native_matrix.py').read_text().split('actual=[]')[0]
    globals()['__name__']='__main__'
    exec(compile(x,'source setup','exec'),globals())
    q=' def prep(*_):'
    ff=fn.replace(q,' pre=[]\n def prep(*_):')
    ff=ff.replace('  if relative>=33075-16:',"""  pre.append((relative,e.r32(v+0x340),struct.unpack('<d',e.rd(v+0x308,8))[0]))
  if relative>=33075-16:""")
    ff=ff.replace(" if variant=='poly':d.set_param(41,0,True)"," if variant=='duration':d.set_param(15,1.25,True)")
    ff=ff.replace("return dict(who=who","return dict(pre=pre,who=who")
    ns=dict(globals());exec(compile('def alt('+ff,'source standalone witness','exec'),ns)
    out=[]
    for name,lead,variant in [('held',0,'default'),('midrelease',11025,'default'),('released',22050,'default'),('staggered',11025,'staggered'),('duration',11025,'duration')]:
        res=ns['alt']('sampler1',lead,variant)
        counts=[sum(t<33075-lead for t,k,y in res['pre']),sum(33075-lead<=t<33075 for t,k,y in res['pre'])]
        count=lambda n:(n+15)//16
        assert counts==[count(33075-lead),count(lead)]
        p=dict(cfg['sampler_control_writes']['sampler1'])
        if variant=='duration':p[15]=1.25
        y,calls=rebuild(p,counts[0],counts[1] if lead else None)
        pre=res['pre']
        anchor=next(y for t,k,y in pre if t==33075 and k==1)
        assert y==anchor,(name,p,counts,y.hex(),anchor.hex(),pre[:12])
        out.append(dict(case=name,ticks=counts,expected=anchor.hex(),actual=y.hex(),
                    event_fmas=calls,commit=res['commit'][0][0]))
    print(json.dumps(out,indent=2))
if __name__=='__main__':main()
