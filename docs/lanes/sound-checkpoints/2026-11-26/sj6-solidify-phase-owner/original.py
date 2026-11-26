"""Execute current genuine sampler at controlled mono/legato gate transitions."""
from pathlib import Path
s=Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0]
exec(compile(s,'verified current original receiver','exec'))
cue={
    "fresh_chord":{16384:[(36,87)],38434:[(36,0)],49459:[(48,87),(52,87),(55,87)],82534:[(48,0),(52,0),(55,0)]},
    "held_then_legato":{16384:[(36,87)],49459:[(48,87),(52,87),(55,87)],66150:[(36,0),(48,0),(52,0)],82534:[(55,0)]},
}
def go(label,ev):
    e=Emu(str(BIN));d=SamplerDSP(e,44100); d.load_placeholder()
    for p,k in cfg['sampler_control_writes']['oscSampler1'][:57]:d.set_param(p,k,True)
    for p,k in ((0,.808),(28,1),(29,1),(25,0),(51,0),(36,20000.),(7,0),(8,0),(40,0),(1,0),(2,0),(3,0),(37,440.),(38,1),(39,1),(41,0),(42,0),(43,1),(44,0),(13,1),(10,.002),(12,.002),(15,.012720)):
        d.set_param(p,k,True)
    e.call(0x1003898c4,d.dsp,4)
    e.call(0x1003898a4,d.dsp,0)
    # Stop at first source-prep of exact clock after each event; read live values,
    # not input phases inferred from output. Omit private waveforms.
    now=0;seen=set();rows=[]
    def rd(*_):
      v=e.x(0)
      if e.r32(v+0x340) and now in ev and now not in seen:
       seen.add(now)
       rows.append(dict(frame=now,hit='before_first_prep_after_events',stage=e.r32(v+0x340),
         phase=float(e.rf(v+0x52c)),key=e.r32(v+0x4c8),freq=float(e.rf(v+0x4d0))))
    e.uc.hook_add(UC_HOOK_CODE,rd,begin=0x1003b4c54,end=0x1003b4c54)
    eventlist=sorted(ev)
    while now<87000:
      for k,v in ev.get(now,[]):e.call(0x100389c84 if v else 0x100389c8c,d.dsp,k,v,doubles=(0.,))
      onward=next((f for f in eventlist if f>now),999999)
      n=min(128,onward-now)
      d.render(n);now+=n
    assert not e.unknown and len(seen)==len(ev)
    return rows
ret={k:go(k,v) for k,v in cue.items()}
# The true source does not rewind during a live held legato or final release.
print(json.dumps(dict(binary=pin(BIN),scope='Authentic executable, neutral controlled generated Mono+Legato with same ranged cue landmarks; not active factory Vintage entropy or terminal AU.',cases=ret),indent=2))
