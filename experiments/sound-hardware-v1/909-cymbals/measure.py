#!/usr/bin/env python3
"""Compare same-length IMA streams to the repository's uncompressed 909 cymbals."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[3]
sys.path[:0] = [str(ROOT / "tools"), str(ROOT / "tests/host")]
from cymbal_ref import decode
from gen_drum_samples import read_wav
from sampleio import ima_encode


def digest(data):
    return hashlib.sha256(data).hexdigest()


def snr(signal, residual):
    return float(10 * np.log10(np.sum(signal**2) / np.sum(residual**2)))


def metrics(pcm, data):
    original = np.asarray(pcm, dtype=float)
    decoded = np.asarray(decode(data, len(pcm)), dtype=float)
    error = decoded - original
    block = 1024
    win = np.hanning(block)
    def spectrum(x):
        frames = np.lib.stride_tricks.sliding_window_view(x, block)[::256]
        return np.abs(np.fft.rfft(frames * win, axis=1))
    ref_spectrum = spectrum(original)
    band = (np.fft.rfftfreq(block, 1/44100) >= 20) & (np.fft.rfftfreq(block, 1/44100) <= 20000)
    floor = np.max(ref_spectrum) * 1e-4
    difference = 20 * np.log10(np.maximum(spectrum(decoded), floor) / np.maximum(ref_spectrum, floor))
    return {"adpcm_sha256": digest(data), "bytes": len(data),
            "snr_db": snr(original, error),
            "first_20ms_snr_db": snr(original[:882], error[:882]),
            "after_150ms_snr_db": snr(original[6615:], error[6615:]),
            "spectral_rms_db_20_20000": float(np.sqrt(np.mean(difference[:, band]**2))),
            "global_level_error_db": float(20*np.log10(np.linalg.norm(decoded) / np.linalg.norm(original))),
            "maximum_absolute_error_s16": int(np.max(np.abs(error)))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="fresh output directory")
    parser.add_argument("--one-step", type=Path, help="optional external one-step IMA streams")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    result = {}
    for name in ("hh", "crash", "ride"):
        wav = ROOT / "assets/909" / (name + ".wav")
        pcm, bits = read_wav(wav)
        encoded = (ROOT / "assets/909/encoded" / (name + ".adpcm")).read_bytes()
        greedy = ima_encode(pcm, 0)[0]
        if len(encoded) != len(greedy):
            raise ValueError(f"sample count changed for {name}")
        row = {"wav_sha256":digest(wav.read_bytes()),"source_depth":bits,"frames":len(pcm),
               "stock":metrics(pcm,greedy),"trellis":metrics(pcm,encoded)}
        if args.one_step:
            data = (args.one_step / (name+".adpcm")).read_bytes()
            if len(data) != len(encoded): raise ValueError("one-step stream length differs")
            row["one_step"] = metrics(pcm,data)
        result[name] = row
    (args.output / "result.json").write_text(json.dumps(result,indent=2,allow_nan=False)+"\n")
    for name,row in result.items():
        print(f"{name}: {row['stock']['snr_db']:.3f} -> {row['trellis']['snr_db']:.3f} dB")

if __name__ == "__main__":
    main()
