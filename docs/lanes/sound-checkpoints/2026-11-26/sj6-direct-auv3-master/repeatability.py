"""Estimate source repeatability before attributing residual to firmware."""
import hashlib
import json
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent
MATCH_KEYS = (
    "component", "component_version", "installed_au_sha256", "preset_name",
    "ablation_arm", "muted_parameter_ids", "schedule", "rate", "frames", "warm_frames",
    "s4_input_sha256",
)
WINDOWS = {
    "whole": (0, 148684), "attack": (16384, 18884), "single": (21000, 38000),
    "transition": (49459, 50500), "chord": (55000, 80000), "release": (82534, 148684),
}

def load(name):
    folder = ROOT / name
    receipt = json.loads((folder/"capture.json").read_text())
    pcm = np.load(folder/"output.npy", allow_pickle=False)
    if pcm.shape != (2, 148684) or not np.isfinite(pcm).all():
        raise ValueError(f"{name}: invalid PCM")
    if hashlib.sha256(pcm.astype("<f4").tobytes()).hexdigest() != receipt["pcm_sha256"]:
        raise ValueError(f"{name}: PCM hash mismatch")
    return receipt, pcm.astype(np.float64)

def db(ratio):
    return round(float(20*np.log10(ratio)), 8)

def run():
    result = {}
    for arm in ("main-only", "sub-only"):
        earlier, x = load("repeat-"+arm)
        current, y = load("native-warm-brainwave-master-authored-"+arm)
        for key in MATCH_KEYS:
            if earlier[key] != current[key]:
                raise ValueError(f"{arm}: capture metadata mismatch: {key}")
        result[arm] = {}
        for label, (lo, hi) in WINDOWS.items():
            a, b = x[:,lo:hi], y[:,lo:hi]
            energy = np.linalg.norm(a)
            result[arm][label] = {
                "level_delta_db": db(np.linalg.norm(b)/energy),
                "residual_dbr": db(np.linalg.norm(a-b)/energy),
            }
    return result

if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
