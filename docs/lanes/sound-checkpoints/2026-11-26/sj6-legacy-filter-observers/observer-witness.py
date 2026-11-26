"""Run only inside the intact current original in-process bench workspace."""
from pathlib import Path

setup = Path("sj6-current-faders/probe.py").read_text()
assert setup.count("cases={}") == 1
exec(compile(setup.split("cases={}")[0], "current original setup", "exec"))
from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE

e.w64(0x1007e3be8, 0x658)
samplers = [e.r64(core + e.r64(d[3])) for d in defs]
assert len(samplers) == 3 and all(samplers)
e.w64(core + 0x658, w.native_array([samplers[2], samplers[0], samplers[1]]))
ranges = tuple([*samplers, *dsp, core, cond, inst])
codes, writes, traces = [], [], []

def code(uc, pc, size, data):
    if 0x1000f9518 <= pc <= 0x1000f9614:
        codes.append(pc)

def store(uc, access, address, size, value, data):
    if any(address < b + 0x500 and address + size > b for b in ranges):
        writes.append((address, size, value))

e.uc.hook_add(UC_HOOK_CODE, code)
e.uc.hook_add(UC_HOOK_MEM_WRITE, store)
for value in (0.0, 0.25, 0.75, 1.0):
    codes.clear()
    writes.clear()
    before = len(w.msglog)
    e.call(0x1000f9614, floats=(value,), regs={20: cond})
    assert not writes and len(w.msglog) == before and not e.unknown
    assert len(codes) == 65 and codes.count(0x1000f95ac) == 3
    traces.append(tuple(codes))
assert all(t == traces[0] for t in traces)
print("4 values, identical 65-instruction observer trace, 3 real iterator advances, no DSP writes")
