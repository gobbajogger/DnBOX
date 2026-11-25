"""Isolated SuperJ6 stock delay: matched tone probes, no output normalization."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

import numpy as np

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("bench", type=Path, help="sj6-fx-bench/bench.py")
p.add_argument("bridge", type=Path, help="compiled compact bridge")
p.add_argument("reference", type=Path, help="verified reference source directory")
p.add_argument("source", type=Path, help="compact juno6_delay_native.inc")
a = p.parse_args()
spec = importlib.util.spec_from_file_location("effects", a.bench)
bench = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bench)
rate = bench.SR
length = 4096
tap = rate // 4
t = np.arange(length) / rate
env = np.sin(np.pi * np.arange(length) / length) ** 2
rate_status = {"probe_rate": rate, "tap_frames": tap, "input": "left-only sine burst, 0.25 peak, sine-squared window, x0.99*64/127 feedback", "compact_source_sha256": hashlib.sha256(a.source.read_bytes()).hexdigest()}
def analyze(fn):
    dry, wet = (fn(v) for v in (False, True))
    result = {"exact_bypass": np.array_equal(dry, x), "max_right": float(np.max(np.abs(wet[1]))), "echoes": [], "first_echo_peak_hz": None}
    for n in (1,2,3,4):
        echo = (wet-dry)[0,n*tap:n*tap+length]
        phasor = np.dot(echo, np.exp(-2j*np.pi*f*t))
        result["echoes"].append(phasor)
        if n == 1:
            fft_size = 32768
            peak = np.argmax(np.abs(np.fft.rfft(echo, n=fft_size)))
            result["first_echo_peak_hz"] = round(float(peak) * rate / fft_size, 1)
    return result
rows = []
for f in (220, 1000, 3000, 6000):
    x = np.zeros((2, tap * 4 + length + 1024), dtype=np.float32)
    x[0,:length] = .25 * env * np.sin(2*np.pi*f*t)
    r = analyze(lambda v: bench.audited_delay_reference(x, v, a.reference))
    c = analyze(lambda v: bench.compact(a.bridge, 'delay', x, v))
    ratio = np.array(c.pop("echoes")) / r.pop("echoes")
    rows.append({"hz":f, "reference":r, "compact":c,
                 "level_error_db":(20*np.log10(np.abs(ratio))).round(4).tolist(),
                 "phase_error_deg":np.angle(ratio, deg=True).round(3).tolist()})
print(json.dumps({**rate_status, "probes":rows}, indent=2, allow_nan=False))
