#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Judge tests/host/render_timing_regression_test.sh runs (CSV from tests/host/audio_stress_test.c).

Per block: cpu_ns = the whole render (both 256-frame stretches), synth A / B = the two synth parts'
original_runtime time inside it, rest = cpu_ns - A - B (drums, BREAK, FX bus, master: code this
check uses as the same-run speed reference). The first WARMUP blocks are skipped.

Fails when:
  - any block's cpu_ns exceeds HALF_DEADLINE_NS / SAFETY (a late block with a safety factor);
  - a synth part's mean / rest-mean ratio exceeds the baseline by more than MEAN_TOL;
  - the block max / rest-mean ratio exceeds the baseline by more than MAX_TOL.
Repetitions of a pair take the best (lowest) ratio, so one preempted run cannot fail the check.
--write-baseline records the ratios instead of judging them.
"""
import argparse, csv, json, sys

HALF_DEADLINE_NS = 11_609_000      # 512 frames at 44.1 kHz (main_fm1.c HALF_FRAMES)
SAFETY = 3                          # host block must stay under a third of the device deadline
WARMUP = 16
MEAN_TOL = 0.30
MAX_TOL = 1.00
NAMES = {"0": "chicago", "1": "miami", "2": "synthone", "3": "superj6"}


def measure(path):
    rows = list(csv.DictReader(open(path)))[WARMUP:]
    if len(rows) < 64:
        raise SystemExit(f"render-timing: {path}: {len(rows)} blocks after warm-up (< 64)")
    cpu = [int(r["cpu_ns"]) for r in rows]
    a = [int(r["chicago_cpu_ns"]) for r in rows]      # synth part A (column name from the stress CSV)
    b = [int(r["miami_cpu_ns"]) for r in rows]        # synth part B
    rest = [c - x - y for c, x, y in zip(cpu, a, b)]
    rm = sum(rest) / len(rest)
    return {"blocks": len(rows), "max_ns": max(cpu), "mean_ns": sum(cpu) / len(cpu),
            "a_mean_ns": sum(a) / len(a), "b_mean_ns": sum(b) / len(b), "rest_mean_ns": rm,
            "a_ratio": sum(a) / len(a) / rm, "b_ratio": sum(b) / len(b) / rm, "max_ratio": max(cpu) / rm}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", required=True)
    ap.add_argument("--runtime-opt", default="?")
    ap.add_argument("--write-baseline", action="store_true")
    ap.add_argument("runs", nargs="+", help="pair:csv, e.g. 0,1:build/x.csv")
    a = ap.parse_args()
    limit = HALF_DEADLINE_NS // SAFETY
    pairs, bad = {}, []
    for spec in a.runs:
        pair, path = spec.split(":", 1)
        m = measure(path)
        late = sum(1 for r in list(csv.DictReader(open(path)))[WARMUP:] if int(r["cpu_ns"]) > limit)
        print(f"render-timing: pair={pair} {path}: blocks={m['blocks']} mean={m['mean_ns']/1e3:.1f}us "
              f"max={m['max_ns']/1e3:.1f}us late={late} (limit {limit/1e3:.1f}us = {HALF_DEADLINE_NS/1e3:.0f}/{SAFETY}) "
              f"{NAMES[pair[0]]}={m['a_mean_ns']/1e3:.1f}us {NAMES[pair[2]]}={m['b_mean_ns']/1e3:.1f}us rest={m['rest_mean_ns']/1e3:.1f}us")
        if late:
            bad.append(f"pair {pair}: {late} late blocks over {limit/1e3:.1f} us")
        best = pairs.setdefault(pair, dict(m))
        for k in ("a_ratio", "b_ratio", "max_ratio"):
            best[k] = min(best[k], m[k])
    if a.write_baseline:
        out = {"note": "render_timing_check.py ratios (engine mean / rest mean, block max / rest mean); "
                       "best of the repetitions", "runtime_opt": a.runtime_opt,
               "pairs": {p: {k: round(v[k], 4) for k in ("a_ratio", "b_ratio", "max_ratio")} for p, v in sorted(pairs.items())}}
        open(a.baseline, "w").write(json.dumps(out, indent=1) + "\n")
        print(f"render-timing: baseline written to {a.baseline}")
        return 0
    base = json.load(open(a.baseline))["pairs"]
    for pair, m in sorted(pairs.items()):
        if pair not in base:
            bad.append(f"pair {pair}: no baseline"); continue
        for k, tol in (("a_ratio", MEAN_TOL), ("b_ratio", MEAN_TOL), ("max_ratio", MAX_TOL)):
            lim = base[pair][k] * (1 + tol)
            who = {"a_ratio": NAMES[pair[0]] + " mean", "b_ratio": NAMES[pair[2]] + " mean", "max_ratio": "block max"}[k]
            status = "ok" if m[k] <= lim else "REGRESSED"
            print(f"render-timing: pair={pair} {who}: ratio {m[k]:.3f} (baseline {base[pair][k]:.3f}, limit {lim:.3f}) {status}")
            if m[k] > lim:
                bad.append(f"pair {pair} {who}: ratio {m[k]:.3f} > {lim:.3f}")
    if bad:
        for b in bad:
            print("FAIL render-timing:", b)
        return 1
    print(f"PASS render-timing: runtime -{a.runtime_opt}, pairs {','.join(sorted(pairs))}, 0 late blocks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
