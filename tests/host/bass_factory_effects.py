#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Replay the original AU factory manifests through the shipping Chicago and Miami parts.

Set CHICAGO_AU_REFS and MIAMI_AU_REFS to the original full-length factory WAV
directories to score them. Without recordings, the report measures the production
effect against its own bypass control. Use --require-originals for the AU gate.
"""
import argparse
from collections import defaultdict
import hashlib
import json
import os
from pathlib import Path

import numpy as np
from scipy.io import wavfile
from scipy.signal import resample_poly

from bass_reference_qualification import ROOT, compile_renderer, render
from chicago_au_score import compare_pair

# Normalized AU selector order, independent of the product UI's preserved indices.
CHICAGO_INSERTS = ("off", "peaking_eq", "compressor", "decimator", "ring_mod", "chorus_flanger",
                   "ensemble", "phaser", "short_delay", "bpm_delay", "hall", "gate_reverb")


def rms(audio):
    return np.sqrt(np.mean(np.square(audio, dtype=np.float64), axis=0))


def digest(pcm):
    return hashlib.sha256(pcm.astype("<f4", copy=False).tobytes()).hexdigest()


def factory_render(exe, engine, scenario, action):
    sr = scenario["sampleRate"]
    if sr != 48000:
        raise ValueError(f"Unexpected AU sample rate {sr}")
    def native(frame):
        return round(frame * 44100 / sr)
    events = ((native(scenario["noteOnFrame"]), 144, scenario["midiNote"], scenario["velocity"]),
              (native(scenario["noteOffFrame"]), 128, scenario["midiNote"], 0))
    controls = ["--factory", scenario["bpm"], "arp-off" if scenario.get("arpOff") else "stored", action]
    return render(exe, engine, scenario["factoryPresetName"], controls, native(scenario["frames"]), events)


def compare_reference(path, scenario, wet):
    sr = scenario["sampleRate"]
    file_rate, reference = wavfile.read(path)
    if file_rate != sr or reference.dtype != np.float32 or not np.isfinite(reference).all():
        raise ValueError(f"{path}: expected finite 32-bit float audio at {sr} Hz")
    reference = reference.astype(np.float64)
    candidate = resample_poly(wet.astype(np.float64), 160, 147, axis=0)
    if reference.shape != candidate.shape or len(candidate) != scenario["frames"]:
        raise ValueError(f"{path}: expected {scenario['frames']} full-length stereo frames")
    channels = {}
    for i, side in enumerate(("left", "right")):
        score = compare_pair(reference[:, i:i+1], candidate[:, i:i+1], scenario["nominalHz"], sr,
                             scenario["steadyWindowStart"], scenario["steadyWindowEnd"])
        cents = score["pitchErrorCents"]
        channels[side] = dict(pitch_error_cents=cents if np.isfinite(cents) else None,
                              steady_envelope_db=score["steadyEnvelopeErrorDbMax"],
                              strong_relative_harmonics_db=score["strongRelativeHarmonicMaxDb"],
                              steady_lag_aligned_correlation=score["lagAlignedCorrelation"],
                              goals=score["goals"])
    release = scenario["noteOffFrame"]
    return dict(wav_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), channels=channels,
                full_stereo_error_rms=rms(candidate - reference).tolist(),
                tail_stereo_error_rms=rms(candidate[release:] - reference[release:]).tolist(),
                pass_goals=all(all(side["goals"].values()) for side in channels.values()))



def run_engine(exe, engine, ref_dir):
    bank = json.loads((ROOT / "assets/factory_banks" / f"{engine}.json").read_text())
    programs = bank["presets"]
    paths = sorted((ROOT / "tests/scenarios" / f"{engine}_au").glob(f"{engine}_au_*.json"))
    if len(paths) != len(programs) or len(programs) != {"chicago": 81, "miami": 30}[engine]:
        raise ValueError(f"{engine} has an incomplete factory bank")
    rows = []
    seen = set()
    by_effect = defaultdict(lambda: {"factory_programs": 0, "different_from_bypass": 0})
    for path in paths:
        scene = json.loads(path.read_text())
        name = scene["factoryPresetName"]
        program = programs[scene["factoryPresetIndex"]]
        if name != program["name"] or name in seen:
            raise ValueError(f"Factory index/name mismatch for {path}")
        seen.add(name)
        if engine == "chicago":
            effect = CHICAGO_INSERTS[round(program["params"][17] * 11)]
        else:
            effect = "crush" if program["params"][10] != 0 else "off"
        wet = factory_render(exe, engine, scene, "stored")
        dry = factory_render(exe, engine, scene, "bypass")
        if not np.all(rms(wet) > 0):
            raise ValueError(f"{name}: silent channel")
        different = not np.array_equal(wet, dry)
        counts = by_effect[effect]
        counts["factory_programs"] += 1
        counts["different_from_bypass"] += int(different)
        row = dict(name=name, effect=effect, candidate_sha256=digest(wet), bypass_sha256=digest(dry),
                   wet_bypass_stereo_error_rms=rms(wet.astype(np.float64) - dry).tolist())
        reference = ref_dir / scene["refWav"] if ref_dir is not None else None
        if reference is not None and reference.is_file():
            row["original_au"] = compare_reference(reference, scene, wet)
        rows.append(row)
    expected = set(CHICAGO_INSERTS if engine == "chicago" else ("off", "crush"))
    if set(by_effect) != expected:
        raise ValueError(f"{engine}: missing factory effect selectors")
    bypass_ok = by_effect["off"]["different_from_bypass"] == 0
    wet_ok = all(row["different_from_bypass"] > 0 for effect, row in by_effect.items() if effect != "off")
    measured = [r["original_au"] for r in rows if "original_au" in r]
    summary = dict(factory_programs=len(rows), finite_and_audible_stereo=len(rows), by_effect=dict(by_effect),
                   bypass_control_pass=bypass_ok and wet_ok, original_au_pairs=len(measured),
                   original_au_pass=sum(r["pass_goals"] for r in measured))
    return summary, rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--require-originals", action="store_true")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    exe = compile_renderer(args.out.resolve())
    summaries = {}
    passed = True
    for engine in ("chicago", "miami"):
        env = os.environ.get(f"{engine.upper()}_AU_REFS")
        refs = Path(env) if env else None
        if refs is not None and not refs.is_dir():
            parser.error(f"Missing {engine} original AU directory: {refs}")
        summary, rows = run_engine(exe, engine, refs)
        summary_file = args.out / f"{engine}-factory-effects.json"
        summary_file.write_text(json.dumps(dict(summary=summary, programs=rows), indent=2, allow_nan=False) + "\n")
        summary["program_results_sha256"] = hashlib.sha256(summary_file.read_bytes()).hexdigest()
        summaries[engine] = summary
        passed &= summary["bypass_control_pass"] and summary["original_au_pairs"] == summary["original_au_pass"]
        if args.require_originals or refs is not None:
            passed &= summary["original_au_pairs"] == summary["factory_programs"]
    text = json.dumps(summaries, indent=2, allow_nan=False) + "\n"
    (args.out / "summary.json").write_text(text)
    print(text, end="")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
