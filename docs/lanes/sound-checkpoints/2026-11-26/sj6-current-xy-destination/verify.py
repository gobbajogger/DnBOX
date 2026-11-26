from pathlib import Path

p = Path('sj6-current-faders/probe.py')
exec(compile(p.read_text().split('cases={}')[0], str(p), 'exec'))
e.w64(0x10082f390, 0x570)
e.w64(0x10082f398, 0x571)
for x in [-1., *map(float, range(13)), 16., 32., .999, 4.99]:
    values = []
    for pc, off in [(0x1000f160c, 0x570), (0x1000f1684, 0x571)]:
        n = len(w.msglog)
        e.call(pc, floats=(x,), regs={20:cond})
        assert len(w.msglog) == n and not e.unknown
        values.append(e.rd(cond+off, 1)[0])
    expected = [int(x)] * 2 if 0 <= int(x) <= 9 else [4, 2]
    assert values == expected, (x, values)
for i, expected in enumerate((0x4290, 0x1b1a, 0x0807, 0x5958, 0x6160,
                              0x6564, 0x0d0f, 0x4948, 0x6768, 0x3c3d)):
    e.call(0x1000e85f8, regs={0:i})
    assert e.x(0) == expected and not e.unknown, (i, e.x(0))
