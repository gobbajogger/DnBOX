"""Private current Sine carrier calibration/binding; never output raw media."""
from pathlib import Path
s=Path('sj6-modern-owner/probe.py').read_text().split('def trial(')[0]
exec(compile(s,'verified current owner prefix','exec'))
bank_sha='86544b4049376ab7838cc1bf30df4f0701826b3960a930ecd78bcbea7112acc6'
def extract(e):
    addr=(0x1007ed6c8,0x1007ee6f0,0x1007ef718,0x1007f0740,0x1007f1768,0x1007f2790,0x1007f37b8)
    assert all(e.r32(k+16)==1024 for k in addr)
    raw=b''.join(e.rd(k+32,4096) for k in addr)
    assert hashlib.sha256(raw).hexdigest()==bank_sha
    return raw
def init(e,k):
    d=SamplerDSP(e,44100);d.load_placeholder()
    for p,v in cfg['sampler_control_writes']['oscSampler1'][:57]:d.set_param(p,v,True)
    for p,v in ((0,.808),(1,0),(2,0),(3,0),(8,0),(9,0),(10,.002),(12,.002),(13,1),(15,.01272),(20,1),(25,0),(28,1),(29,1),(32,1),(36,20000),(37,440),(38,1),(39,0),(40,0),(41,0),(42,0),(43,1),(44,0),(51,0)):
      d.set_param(p,v,True)
    e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,0)
    raw=extract(e)
    bank=e.calloc(len(raw)); e.wr(bank,raw)
    info=e.calloc(32);e.wr(info,struct.pack('<QIII',bank,1024,7,64));e.call(0x1003898cc,d.dsp,1,info);e.call(0x1003898d4,d.dsp,1);e.call(0x1003898b4,d.dsp,1)
    if k is not None:e.call(0x100389c84,d.dsp,k,87,floats=(0.,))
    return d
