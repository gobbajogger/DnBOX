#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Render Chicago/Miami factory notes and compare both engines to original AU measurements.

Usage: OPENBLAS_NUM_THREADS=1 python3 tests/host/bass_reference_qualification.py --out DIR
Requires numpy and scipy. Returns 1 if any measured goal fails. Chicago includes 29 native-rate
reference hashes and measurements. Miami uses seven cross-rate measurements.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

import numpy as np
from scipy.signal import resample_poly
from scipy.optimize import minimize_scalar

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import build
import check_original_inputs

sys.path.insert(0, str(ROOT / "experiments/wobble"))
import compare as au

REFERENCE = ROOT / "experiments/wobble/evidence/comparison.json"


def compile_renderer(out):
    if not check_original_inputs.verify(
        ROOT, expected_manifest_sha256=check_original_inputs.MANIFEST_SHA256
    )["ok"]:
        raise SystemExit("Original bass input verification failed")
    exe = out / "bass_reference_render"
    subprocess.run([
        os.environ.get("CC", "cc"), "-std=c99", "-O2", "-D_POSIX_C_SOURCE=200809L",
        "-DX0X_HOST", "-DX0X_ORIGINAL_SYNTH=1", "-DX0X_DEV_WOBBLE=0", "-DX0X_WOBBLE=0",
        "-Ifirmware/src", "-Ifirmware/src/dsp", "-Ibuild/gen", "-Iexperiments/original_synth",
        "-Iexperiments/wobble", *build.ORIGINAL_SYNTH_FLAGS, "tests/host/bass_reference_render.c",
        *build.ORIGINAL_SYNTH_UNITS, *build.ORIGINAL_SYNTH_BACKENDS, "firmware/src/dsp/x0x_param.c",
        "-lm", "-o", str(exe),
    ], cwd=ROOT, check=True)
    return exe


def render(exe, engine, program, controls=(), frames=44100,
           events=((0, 144, 48, 80), (22050, 128, 48, 0)), boundaries=()):
    if any(status not in (128, 144) for _, status, _, _ in events):
        raise ValueError("Unexpected MIDI status")
    notes = [(tick, key, 0 if status == 128 else velocity) for tick, status, key, velocity in events]
    payload = [frames, len(notes), len(boundaries), *(v for event in notes for v in event), *boundaries]
    raw = subprocess.check_output([str(exe), engine, str(program), *map(str, controls)],
                                  input=" ".join(map(str, payload)).encode("ascii"))
    if len(raw) != frames * 2 * 4:
        raise ValueError(f"{engine} {program} emitted {len(raw)} bytes")
    pcm = np.frombuffer(raw, dtype=np.float32).reshape(frames, 2)
    if not np.isfinite(pcm).all():
        raise ValueError(f"{engine} {program} emitted non-finite samples")
    return pcm


def compare_miami(exe):
    original = json.loads(REFERENCE.read_text())
    if original["sampleRate"] != 48000 or set(original["configurations"]) != set(au.CONFIGS):
        raise ValueError("Unexpected reference format or incomplete matrix")
    rows = []
    for name in au.CONFIGS:
        record = original["configurations"][name]
        params = sorted(record["parameters"], key=lambda p: p["id"])
        events = [(e["relativeFrame"], e["status"], e["note"], e["velocity"]) for e in record["events"]]
        if ([p["id"] for p in params] != list(range(18))
                or events != [(0, 144, 48, 80), (24000, 128, 48, 0)]
                or not record["referenceRepeatExactStereo"]
                or record["referenceSha256"] != record["replicateSha256"]):
            raise ValueError(f"Invalid reference inputs for {name}")
        src = render(exe, "miami", 0, [p["readback"] for p in params])
        # The original archived measurements used the 48 kHz analysis windows.
        # Resample only for the analysis; no waveform alignment or gain fitting.
        measured = au.measure(resample_poly(src.astype(np.float64), 160, 147, axis=0), record["reference"]["nominalHz"])
        reference = record["reference"]
        cents = float(1200 * np.log2(measured["pitchFitHz"] / reference["pitchFitHz"]))
        env = np.array(measured["envelope50msRms"])[2:8] / np.array(reference["envelope50msRms"])[2:8]
        envelope_db = float(np.max(np.abs(20 * np.log10(env))))
        a, b = (np.array(x["fixedHarmonicAbs"]) for x in (reference, measured))
        mask = (a > a[0] * .01) | (b > b[0] * .01)
        harmonic_db = 20 * np.log10(np.maximum(b, 1e-20) / np.maximum(a, 1e-20))
        spectrum_db = float(np.max(np.abs((harmonic_db - harmonic_db[0])[mask])))
        pitch_valid = reference["pitchFitValid"] and measured["pitchFitValid"]
        rows.append(dict(
            configuration=name, reference_wav_sha256=record["referenceSha256"],
            pitch_error_cents=cents, pitch_valid=pitch_valid, max_steady_envelope_db=envelope_db,
            max_relative_harmonic_db=spectrum_db,
            pass_goals=bool(pitch_valid and abs(cents) <= 2 and envelope_db <= 1 and spectrum_db <= 1),
        ))
    return rows


def compare_chicago(exe):
    packet = json.loads((ROOT / "tests/fixtures/chicago-au-3.1.6.json").read_text())
    if (packet["sample_rate"] != 44100 or packet["au_version"] != "3.1.6"
            or len({r["name"] for r in packet["rows"]}) != 29 or len(packet["rows"]) != 29):
        raise ValueError("Incomplete Chicago reference packet")
    rows = []
    for row in packet["rows"]:
        if len(row["controls"]) != 22 or len(row["harmonics_rel_db"]) != 12:
            raise ValueError(f"Incomplete controls or measurements for {row['name']}")
        # AudioUnit host splits all trials at these edges, even without a note.
        cuts = [i for i in (11025, 16538, 22050, 33075, 44100, 66150) if i < row["frames"]]
        pcm = render(exe, "chicago", 0, row["controls"], row["frames"], row["events"], cuts)
        mono = pcm.astype(np.float64).mean(axis=1)
        start, end = row["fit_window_frames"]
        steady = mono[start:end]
        # Fit pitch in the same per-stimulus window and 1% search range as the AU
        # measurement. Downsample eightfold for the four-harmonic search, then
        # fit twelve harmonics using all original-rate samples.
        note = row["fit_note"]
        nominal = 440 * 2 ** ((note - 69) / 12)
        decimated = resample_poly(steady, 1, 8)
        search_times = np.arange(len(decimated)) * 8 / 44100

        def matrix(f, times, harmonics):
            phase = 2 * np.pi * f * times[:, None] * np.arange(1, harmonics + 1)
            return np.column_stack((np.ones(len(times)), np.cos(phase), np.sin(phase)))

        def loss(f):
            bases = matrix(f, search_times, 4)
            coeff = np.linalg.lstsq(bases, decimated, rcond=None)[0]
            return np.mean((decimated - bases @ coeff) ** 2)

        f0 = minimize_scalar(loss, bounds=(nominal * .99, nominal * 1.01), method="bounded",
                             options={"xatol": 1e-6}).x
        valid = not row["fit_at_boundary"] and min(abs(f0 - nominal * .99), abs(f0 - nominal * 1.01)) >= .001
        tones = matrix(f0, np.arange(len(steady)) / 44100, 12)
        coeff = np.linalg.lstsq(tones, steady, rcond=None)[0]
        amplitude = np.hypot(coeff[1:13], coeff[13:])
        relative = 20 * np.log10(np.maximum(amplitude, 1e-20) / max(amplitude[0], 1e-20))
        reference = np.array(row["harmonics_rel_db"])
        strong = (relative > -40) | (reference > -40)
        spectrum_db = float(np.max(np.abs(relative[strong] - reference[strong])))
        rms = np.sqrt(np.mean(steady**2))
        rms_db = 20 * np.log10(max(rms, 1e-20) / row["rms"])
        envelope = np.sqrt(np.mean(mono[4410:17640].reshape(6, 2205) ** 2, axis=1))
        envelope_db = float(np.max(np.abs(20*np.log10(np.maximum(envelope, 1e-20) / row["envelope_rms"]))))
        cents = float(1200 * np.log2(f0 / row["f0"]))
        digest = hashlib.sha256(pcm.astype("<f4", copy=False).tobytes()).hexdigest()
        rows.append(dict(configuration=row["name"], stereo_pcm_exact=digest == row["raw_sha256"],
                         candidate_sha256=digest, steady_rms_db=float(rms_db), pitch_valid=bool(valid),
                         pitch_error_cents=cents if valid else None,
                         max_steady_envelope_db=envelope_db, max_relative_harmonic_db=spectrum_db,
                         pass_goals=bool(valid and abs(cents) <= 2 and envelope_db <= 1 and spectrum_db <= 1)))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    exe = compile_renderer(out)
    factory = {}
    for engine, total in (("chicago", 81), ("miami", 30)):
        peaks = [np.max(abs(render(exe, engine, i)), axis=0).tolist() for i in range(total)]
        factory[engine] = {"presets": total, "silent_channel_rows": [i for i, p in enumerate(peaks) if min(p) == 0],
                           "minimum_channel_peak": min(map(min, peaks)), "maximum_channel_peak": max(map(max, peaks))}
    matrix = compare_miami(exe)
    chicago = compare_chicago(exe)
    report = {"factory_note": factory, "miami_au_3_1_6_measured": matrix,
              "chicago_au_3_1_6_measured": chicago,
              "firmware_hz": 44100, "chicago_au_hz": 44100, "miami_au_hz": 48000,
              "factory_stimulus": {"note": 48, "velocity": 80, "note_off_frame": 22050, "frames": 44100},
              "limits": {"pitch_cents": 2, "steady_envelope_db": 1, "relative_strong_harmonics_db": 1}}
    text = json.dumps(report, indent=2, allow_nan=False) + "\n"
    (out / "report.json").write_text(text)
    print(text, end="")
    return int(any(x["silent_channel_rows"] for x in factory.values()) or not all(r["pass_goals"] for r in matrix + chicago))


if __name__ == "__main__":
    sys.exit(main())
