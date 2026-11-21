#!/usr/bin/env python3
"""Measure the independent oscillator against the pinned original waveforms and program fields."""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

import numpy as np

HERE = Path(__file__).resolve().parent
RMS_LIMIT, SPECTRAL_LIMIT = 1.0, 3.0
EDGE = np.array([0, 8.178783416748047, 16.357566833496094, 32.71513366699219,
                 65.43026733398438, 131.25, 262.5, 525, 1050, 2205, 4410,
                 11025, 22050], dtype=np.float32)
PRIOR_GRID = [32.7, 32.73, 65.42, 65.44, 131.24, 131.26, 261.6256, 262.49, 262.51,
              440, 524.99, 525.01, 1049.99, 1050.01, 2204.99, 2205.01,
              4410, 11024.99, 11025.01]
TIMED = [5, 8.17, 8.19, 12.34, 16.35, 16.37, 22.31, 32.73, 60.25, 131.26,
         441, 2205.01, 11025.01]


def verified(path, expected):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != expected:
        raise ValueError(f"Wrong bytes for {path}")
    return data


def metrics(reference, actual):
    reference, actual = reference.astype('f8'), actual.astype('f8')
    if not np.isfinite(reference).all() or not np.isfinite(actual).all():
        raise ValueError("Nonfinite audio")
    a, b = abs(np.fft.rfft(reference)), abs(np.fft.rfft(actual))
    floor = max(a.max() * 1e-4, 1e-12)
    mask = (a >= floor) | (b >= floor)
    db = 20 * np.log10(np.maximum(a[mask], floor) / np.maximum(b[mask], floor))
    rms = abs(10 * np.log10(np.mean(actual**2) / np.mean(reference**2)))
    return float(rms), float(np.sqrt(np.mean(db**2)))


def score(rows):
    return {"pass": sum(r <= RMS_LIMIT and s <= SPECTRAL_LIMIT for r, s in rows),
            "total": len(rows), "worst_rms_db": max(r for r, _ in rows),
            "worst_spectral_db": max(s for _, s in rows)}


def run(args):
    binary = verified(args.original, "0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5")
    table = np.frombuffer(binary, '<f4').reshape(13, 4, 4097)[:, :, :4096].astype('f8')
    commands = verified(args.commands, "25a9c436f4dc7c37517c77acb632ccdf0753a885ee474e1e8f93c02b0c099a2e")
    programs = [dict(case["commands"]) for case in json.loads(commands)["cases"]]
    verified(args.dsp / "fastmath.h", "c82737d5302d77b974d5071c9bc58915aca723a6e6f5f5185cbe43bad962189d")

    with tempfile.TemporaryDirectory() as temp:
        libpath = Path(temp) / "libprobe.so"
        subprocess.run(["cc", "-std=c99", "-O2", "-fPIC", "-shared", "-ffp-contract=off",
                        "-fno-fast-math", "-Wall", "-Wextra", "-Werror", "-I", str(args.dsp),
                        str(HERE / "band_morph.c"), str(HERE / "host_probe.c"), "-o", str(libpath)], check=True)
        lib = ctypes.CDLL(str(libpath))
        floats = np.ctypeslib.ndpointer(dtype=np.float32, flags="C_CONTIGUOUS")
        lib.sample_cycle.argtypes = [ctypes.c_float]*3 + [ctypes.c_uint, floats]
        lib.sample_steps.argtypes = lib.sample_cycle.argtypes + [floats]
        lib.sample_raw_cycle.argtypes = [ctypes.c_float, ctypes.c_uint, floats]
        lib.sample_parts.argtypes = [ctypes.c_uint, ctypes.c_uint, floats]
        for func in [lib.sample_cycle, lib.sample_steps, lib.sample_parts, lib.sample_raw_cycle]:
            func.restype = None

        def reference(hz, morph, bandlimited=True):
            band = np.searchsorted(EDGE, np.float32(hz)) if bandlimited else 0
            position = np.float32(morph) * np.float32(3)
            idx, frac = int(position), float(position % 1)
            if idx == 3:
                return table[band, idx]
            return (1-frac)*table[band, idx] + frac*table[band, idx+1]

        cycle = np.empty(4096, 'f4')
        cache = {}

        def grid_case(hz, morph, bandlimited=True):
            key = (float(np.float32(hz)), float(np.float32(morph)), bandlimited)
            if key not in cache:
                if bandlimited:
                    lib.sample_cycle(*key[:2], .125, len(cycle), cycle)
                else:
                    lib.sample_raw_cycle(key[1], len(cycle), cycle)
                cache[key] = metrics(reference(*key), cycle)
            return cache[key]

        prior = [grid_case(f, q) for f in PRIOR_GRID for q in np.linspace(0, 1, 7)]
        hz_set = {f for high in EDGE[1:] for f in
                  [np.nextafter(high, np.float32(0)), high, np.nextafter(high, np.float32(np.inf))]
                  if f <= 22050}
        hz_set.update([5, 20, 40, 261.6256, 440])
        edges = [grid_case(f, q) for f in sorted(hz_set) for q in np.linspace(0, 1, 13)]
        factory = [grid_case(hz, p[ident], bool(p[146])) for p in programs for ident, hz in [(0, 261.6256), (1, 440)]]

        phases = np.empty(22050, 'f4')
        output = np.empty_like(phases)
        window = np.hanning(len(output))

        def stepped_case(hz, morph):
            lib.sample_steps(hz, morph, .125, len(output), output, phases)
            ref = reference(hz, morph)
            position = phases.astype('f8') * 4096
            idx = position.astype(int)
            frac = position - idx
            interp = ref[idx]*(1-frac) + ref[(idx+1) % 4096]*frac
            return metrics(interp * window, output * window)

        stepped = [stepped_case(f, q) for f in TIMED for q in np.linspace(0, 1, 7)]
        # Preserve the earlier fixture's independently advancing double phase
        # as an additional reference for the four ordinary pitches.
        prior_stepped = []
        for hz, morph in [(261.6256, 1/6), (440, .5), (329.6276, 4/6), (523.2511, 1)]:
            lib.sample_steps(hz, morph, .125, len(output), output, phases)
            phase = np.arange(len(output), dtype='f8') * np.float32(np.float32(hz)/44100)
            position = (phase % 1)*4096
            idx = position.astype(int)
            frac = position-idx
            ref = reference(hz,morph)
            prior_stepped.append(metrics((ref[idx]*(1-frac)+ref[(idx+1)%4096]*frac)*window,
                                         output*window))

        parts = np.zeros((5, 22050, 4), dtype=np.float32)
        for mode, samples in enumerate(parts):
            lib.sample_parts(mode, len(samples), samples)
        if not np.isfinite(parts).all():
            raise ValueError("Nonfinite edit fixture")
        for mode, changed in [(1, 0), (2, 1), (3, 1)]:
            different = parts[0] != parts[mode]
            if different[:11025].any() or np.delete(different, changed, axis=1).any():
                raise ValueError("A field edit affected an unrelated sample or oscillator")
            if np.count_nonzero(different[:, changed]) <= 1000:
                raise ValueError("Edited oscillator did not respond")
        if not np.array_equal(parts[4, :, :2], parts[0, :, :2]) or parts[4, :, 2:].any():
            raise ValueError("Muted peer changed the first owner")

    groups = {"prior_grid": score(prior), "band_edges": score(edges), "prior_steps": score(prior_stepped),
              "phase_matched_steps": score(stepped), "factory_morph_fields": score(factory)}
    return {"groups": groups, "all_pass": all(x["pass"] == x["total"] for x in groups.values()),
            "factory_programs": len(programs),
            "factory_bandlimited": sum(p[146] == 1 for p in programs),
            "factory_raw": sum(p[146] == 0 for p in programs),
            "distinct_factory_field_values": [len({p[i] for p in programs}) for i in [0, 1]],
            "two_owner_edit_and_mute_checks": 4, "limits_db": [RMS_LIMIT,SPECTRAL_LIMIT],
            "fft_floor_db": -80, "sample_rate_hz": 44100, "intact_program_renders": 0}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for flag in ["original", "commands", "dsp"]:
        parser.add_argument("--" + flag, type=Path, required=True)
    result = run(parser.parse_args())
    print(json.dumps(result, indent=2, allow_nan=False))
    raise SystemExit(0 if result["all_pass"] else 1)
