from loaded import *
from collections import defaultdict
# Source masks supplied as fixed controlled semantics, not recovered captured RNG.
src=json.load(open('sj6-native-vintage-owner/events.json'))['vintage'][:4]
events={16384:[(36,1,0.)],38434:[(36,0,0.)],
        49459:[(48,1,0.),(52,1,0.),(55,1,0.)],
        82534:[(48,0,0.),(52,0,0.),(55,0,0.)]}
def run(which,per_event):
    e=Emu(str(BIN)); d=init(e,None); now=0
    ledger=[]
    intended={r['key']:next(x[2] for x in r['physical_source_calls'] if x[1]==which) for r in src}
    at=0; first={}
    def tap(*_):
      if now not in first and now in (16384,38434,49459,49587,82406,82534,82662):
        first[now]=dict(phase=e.s(10),quarter_increment=e.s(9),phase_hz=float(e.s(9)*4*44100))
    e.uc.hook_add(UC_HOOK_CODE,tap,begin=0x1003b6874,end=0x1003b6874)
    breaks=sorted(set(events)|(set((49587,82406,82662))))
    out=[]
    while now<84000:
        for k,on,_ in events.get(now,[]):
            if on:
                sal=intended[k if per_event or k==36 else 48]
                e.call(0x100389c84,d.dsp,k,87,doubles=(sal,))
                ledger.append((now,k,sal))
            else:e.call(0x100389c8c,d.dsp,k,0)
        n=min(128,next((br for br in breaks if br>now),999999)-now,84000-now)
        out.append(d.render(n)[0]); now+=n
    assert not e.unknown and len(first)==7
    return ledger,first,np.concatenate(out)
rr={}
for osc in (1,2):
    n,a,x=run(osc,False)
    m,b,y=run(osc,True)
    assert x.shape==y.shape==(84000,) and np.array_equal(x[:49459],y[:49459])
    # Compare indefinitely signed sample streams only, never phase-fit.
    s={}
    for name,i,j in [('same_frame_chord',49459,49587),('chord_hold',53869,82534),('final_early',82534,83239)]:
      u,v=x[i:j].astype('f8'),y[i:j].astype('f8')
      d=np.linalg.norm(u-v)/np.linalg.norm(v)
      s[name]=float(20*np.log10(d)) if d else None
    rr[osc]={'sustain_retained':a,'per_event':b,'sustain_retained_input':n,'per_event_input':m,'signed_difference_dbr':s}
print(json.dumps(dict(receipt='Actual current compiled standalone receiver, matched controlled admitted NOTE per-layer detune vs held-gate retain; never claims native saved entropy or conductor routing',shots=rr),indent=2,allow_nan=False))
