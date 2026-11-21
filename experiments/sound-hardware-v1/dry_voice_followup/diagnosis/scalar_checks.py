#!/usr/bin/env python3
"""Bounded scalar algebra checks only; no renderer, audio analysis, or compiler.

All filter equations are independently derived from the permitted frozen B
recurrence. Binary32 helpers reproduce the allowed fastmath coefficient and
Chicago feedback arithmetic for selected scalar points, not a synth render.
"""
import cmath
import hashlib
import json
import math
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
BASE = HERE.parent


def f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def add(a, b):
    return f32(f32(a) + f32(b))


def sub(a, b):
    return f32(f32(a) - f32(b))


def mul(a, b):
    return f32(f32(a) * f32(b))


def div(a, b):
    return f32(f32(a) / f32(b))


def stock_exp2_nf(n, x):
    p = f32(1.534580891e-4)
    for a in (1.339993158e-3, 9.618488972e-3, 5.550328776e-2,
              2.402264689e-1, 6.931472057e-1, 1.0):
        p = add(mul(p, x), a)
    bits = struct.unpack("<I", struct.pack("<f", p))[0]
    bits = (bits + ((n & 0xFFFFFFFF) << 23)) & 0xFFFFFFFF
    return struct.unpack("<f", struct.pack("<I", bits))[0]


def stock_expf(x):
    log2e = f32(1.44269504088896)
    n = math.floor(add(mul(x, log2e), 0.5))
    r = sub(sub(x, mul(n, 0.693145752)), mul(n, 1.42860677e-6))
    return stock_exp2_nf(n, mul(r, log2e))


def b_coefficient(cutoff):
    x = div(mul(-f32(6.28318530717959), cutoff), mul(2.0, 44100.0))
    return stock_expf(x)


def phi(x):
    u = max(0.0, 1.0 - abs(x))
    return math.copysign(1.0 - u ** 5, x)


def phi32(x):
    x = f32(x)
    u = f32(0.0) if abs(x) >= 1.0 else sub(1.0, abs(x))
    u2 = mul(u, u)
    u4 = mul(u2, u2)
    u5 = mul(u, u4)
    y = sub(1.0, u5)
    return y if x >= 0.0 else -y


def exact_root(w, c):
    lo, hi = w - c, w + c
    for _ in range(80):
        mid = (lo + hi) * 0.5
        if mid + c * phi(mid) < w:
            lo = mid
        else:
            hi = mid
    return (lo + hi) * 0.5


def delayed_roots(a, k):
    b = 1.0 - a
    roots = []
    for sign in (-1, 1):
        t = 2.0 * a + sign * 1j * math.sqrt(k) * b * b
        d = cmath.sqrt(t * t - 4.0 * a * a)
        roots.extend(((t + d) * 0.5, (t - d) * 0.5))
    return roots


def centered_roots(a, k):
    g = (1.0 - a) / (1.0 + a)
    result = []
    for j in range(4):
        pole = -1.0 + k ** 0.25 * cmath.exp(1j * (2*j+1) * math.pi / 4.0)
        result.append((1.0 + g * pole) / (1.0 - g * pole))
    return result


cutoff_rows = []
for fc in (64, 500, 2000, 8000, 22050):
    a = b_coefficient(fc)
    critical = (1+a) ** 4 / (1+a*a) ** 2
    row = dict(cutoff_hz=fc, A_binary32=a,
               old_k_critical=critical, old_r_critical=critical/4.0,
               old_unit_circle_crossing_hz=88200.0/math.pi * math.atan((1-a)/(1+a)))
    row["selected_radii"] = [
        dict(resonance=r,
             old=max(abs(z) for z in delayed_roots(a, 4*r)),
             centered=max(abs(z) for z in centered_roots(a, 4*r)))
        for r in (0.0, 0.25, 0.5, 0.9, 0.98)]
    cutoff_rows.append(row)

a_min = math.exp(-math.pi/2)
c_max = (0.8*0.98) * (1-a_min)**4 / 16.0
rho_max = 5*c_max
# Loose bound also allows room for coefficient-rounding error: A >= 0.20.
c_safe = 0.784 * 0.8**4 / 16.0
rho_safe = 5*c_safe

solver_rows = []
# A bounded, declared list of scalar right sides, not a parameter sweep.
right_sides = (0.0, 1e-8, -1e-8, 0.001, -0.001, 0.05, -0.05,
               0.25, -0.25, 0.5, -0.5, 0.98, -0.98,
               1.0, -1.0, 1.02, -1.02, 1.784, -1.784)
for c in (c_max, c_safe):
    for w in right_sides:
        d = w
        for _ in range(6):
            d = w - c * phi(d)
        w32, c32 = f32(w), f32(c)
        d32 = w32
        for _ in range(6):
            d32 = sub(w32, mul(c32, phi32(d32)))
        root = exact_root(w, c)
        root32_inputs = exact_root(w32, c32)
        solver_rows.append(dict(w=w, c=c, real_six=d, real_root=root,
            real_error=abs(d-root), binary32_six=d32,
            binary32_error_against_real_root_for_rounded_inputs=abs(d32-root32_inputs),
            binary32_residual=abs(d32+c32*phi(d32)-w32)))

sources = [
    "s1_dry_voice_unit/src/s1_dry_voice.c",
    "s1_dry_voice_unit/variant_b/src/s1_dry_voice.c",
    "s1_dry_voice_unit/variant_b/src/s1_dry_voice.h",
    "s1_dry_voice_unit/results/ch4_exp_e0/all180_summary.json",
    "s1_dry_voice_unit/variant_b/results/ch4_2x_slope4/all180_summary.json",
    "dry_reference_packet/case_plan.json",
    "dry_reference_packet/metrics.py",
    "DnBOX/experiments/original_synth/chicago_mp4.c",
    "DnBOX/firmware/src/dsp/juno6.c",
    "DnBOX/firmware/src/dsp/fastmath.h",
]
report = dict(
    scope="Scalar equations and selected binary32 arithmetic only; no audio render or gate run",
    original_gate_unchanged=dict(rms_db_max=1, spectral_db_max=3,
                                 comparison_gain=1, nonfinite_max=0),
    cutoff_rows=cutoff_rows,
    solver_bound=dict(A_min_analytic=a_min, c_max_analytic=c_max,
        contraction_max_analytic=rho_max, real_error_after_six_bound=c_max*rho_max**6,
        loose_A_lower_bound=0.20, loose_c_bound=c_safe,
        loose_contraction_bound=rho_safe,
        loose_real_error_after_six_bound=c_safe*rho_safe**6,
        qualification="Real-arithmetic uniform bound; binary32 roundoff is additional"),
    selected_solver_checks=dict(count=len(solver_rows),
        max_real_error=max(r["real_error"] for r in solver_rows),
        max_binary32_error=max(r["binary32_error_against_real_root_for_rounded_inputs"] for r in solver_rows),
        max_binary32_residual=max(r["binary32_residual"] for r in solver_rows),
        rows=solver_rows),
    provenance=[dict(path=str(BASE/p), sha256=hashlib.sha256((BASE/p).read_bytes()).hexdigest())
                for p in sources])
(HERE/"scalar_results.json").write_text(json.dumps(report, indent=2)+"\n")
print(json.dumps({k: report[k] for k in ("scope", "cutoff_rows", "solver_bound")}, indent=2))
print(json.dumps({k: v for k, v in report["selected_solver_checks"].items() if k != "rows"}, indent=2))
