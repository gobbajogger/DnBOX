#!/usr/bin/env python3
"""Bit-exact float history for each public Synth One factory preset.

Reads bank JSON from a directory. This repo does not contain those bodies.
The public banks are AudioKit/AudioKitSynthOne@6466a371 Presets/Data, and
public-source-manifest.json records their SHA-256. Pass that directory.
The private owner archive is not read.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "experiments/synthone-port-feasibility-20261114/public-source-manifest.json"
OWNER = ROOT / "experiments/synthone-port-feasibility-20261114/owner-corpus-manifest.json"
DLY_BUF = 158400
POOL = 0x54000
FLOOR = 8192
POOL_CEILING = POOL - FLOOR
CANVAS = 240 * 138 * 2
UNDO = 4096
SCOPE = 128 * 2
BLE_OQ = 56 * 4
RECLAIM_SEND = DLY_BUF
RECLAIM_KEEP_ENGINES = DLY_BUF + UNDO + SCOPE + BLE_OQ
RECLAIM_KEEP_ENGINES_NO_SCREEN = RECLAIM_KEEP_ENGINES + CANVAS
T_MIN = 0.0003628117914
T_MAX = 2.5
# Note divisions, matching AKSynthOneRate. Delay sync uses twoBars..sixtyFourthTriplet.
RATES = [
    ("eightBars", 8.0, False),
    ("sixBars", 6.0, False),
    ("fourBars", 4.0, False),
    ("threeBars", 3.0, False),
    ("twoBars", 2.0, False),
    ("bar", 1.0, False),
    ("barTriplet", 1.0, True),
    ("half", 0.5, False),
    ("halfTriplet", 0.5, True),
    ("quarter", 0.25, False),
    ("quarterTriplet", 0.25, True),
    ("eighth", 0.125, False),
    ("eighthTriplet", 0.125, True),
    ("sixteenth", 1 / 16, False),
    ("sixteenthTriplet", 1 / 16, True),
    ("thirtySecondth", 1 / 32, False),
    ("thirtySecondthTriplet", 1 / 32, True),
    ("sixtyFourth", 1 / 64, False),
    ("sixtyFourthTriplet", 1 / 64, True),
]


def seconds(bpm, bars, triplet):
    return (4.0 * bars) / (bpm * (1.0 / 60.0)) / (1.5 if triplet else 1.0)


def division_time(bpm, index):
    name, bars, triplet = RATES[index]
    return name, seconds(bpm, bars, triplet)


def nearest_time(stored, bpm):
    """S1Rate::nearestTime. Delay excludes the 8, 6, 4 and 3 bar divisions."""
    if not math.isfinite(bpm) or bpm <= 0:
        bpm = 120.0
    best_i = 18
    best_t = division_time(bpm, best_i)[1]
    best_d = 1e30
    for i in range(18, 3, -1):  # sixtyFourthTriplet down through twoBars
        t = division_time(bpm, i)[1]
        if t < T_MIN or t > T_MAX:
            continue
        d = abs(t - stored)
        if d < best_d:
            best_d = d
            best_i = i
            best_t = t
    return best_t, RATES[best_i][0]


def clamp_time(t):
    if t < T_MIN:
        return T_MIN
    if t > T_MAX:
        return T_MAX
    return t


def line_bytes(seconds_long, sr):
    words = math.ceil(seconds_long * sr - 1e-9) + 4
    if words < 4:
        words = 4
    return words * 4


def delay_bytes(t, sr):
    return 2 * line_bytes(2.0 * t, sr) + 2 * line_bytes(t, sr)


def revsc_bytes(sr):
    params = [
        (2473 / 44100, 0.0010),
        (2767 / 44100, 0.0011),
        (3217 / 44100, 0.0017),
        (3557 / 44100, 0.0006),
        (3907 / 44100, 0.0010),
        (4127 / 44100, 0.0011),
        (2143 / 44100, 0.0017),
        (1933 / 44100, 0.0006),
    ]
    total = 0
    for base, var in params:
        samples = int((base + var * 1.125) * sr + 16.5)
        total += samples * 4
    return total


def widen_bytes(sr):
    return int(round(0.05 * sr)) * 4


def preset_record(bank, item, sr):
    stored = float(item.get("delayTime", 0.5))
    sync = float(item.get("tempoSyncToArpRate", 1.0)) > 0
    bpm = float(item.get("arpRate", 120.0))
    # App load order from a fresh kernel (sync on, 120 BPM): delayTime is
    # snapped at 120 first, then arpRate re-snaps, then the sync switch is set.
    snapped_120, _ = nearest_time(stored, 120.0)
    effective, division = nearest_time(snapped_120, bpm if bpm > 0 else 120.0)
    if not sync:
        division = None
    delay_on = float(item.get("delayToggled", 0.0)) != 0 and float(item.get("delayMix", 0.5)) != 0
    reverb_on = float(item.get("reverbToggled", 0.0)) != 0 and float(item.get("reverbMix", 0.5)) != 0
    widen_on = float(item.get("widen", 0.0)) != 0
    pan_on = float(item.get("autoPanAmount", 0.0)) != 0
    phaser_on = float(item.get("phaserMix", 0.0)) != 0
    ladder_on = float(item.get("filterType", 0.0)) == 0 and delay_on
    delays = delay_bytes(effective, sr)
    rev = revsc_bytes(sr)
    wide = widen_bytes(sr)
    sine = (4096 + 1) * 4
    # Fixed processor state that the extracted frame always runs.
    compressors = 56 * 6
    bitcrush = 16
    hipass = 52 * 2
    phaser = 192 if phaser_on else 0
    moog = 64 * 2 if ladder_on else 0
    pan = sine if pan_on else 0
    heard_delay = delays if delay_on else 0
    heard_rev = rev if reverb_on else 0
    heard_wide = wide if widen_on else 0
    heard = heard_delay + heard_rev + heard_wide + pan + phaser + moog + compressors + bitcrush + hipass
    # Armed: the stored delay time and the reverb tank are allocated, so opening
    # those knobs stays bit-exact. Widen is the fixed 50 ms line.
    armed = delays + rev + wide + pan + phaser + moog + compressors + bitcrush + hipass
    return {
        "bank": bank,
        "name": item.get("name", ""),
        "position": item.get("position"),
        "stored_delay_s": stored,
        "sync": sync > 0,
        "arp_bpm": bpm,
        "division": division,
        "effective_delay_s": effective,
        "delay_on": delay_on,
        "reverb_on": reverb_on,
        "delay_bytes": delays,
        "reverb_bytes": rev,
        "widen_bytes": wide,
        "heard_bytes": heard,
        "armed_bytes": armed,
    }


def histogram(rows, key):
    bins = []
    for i in range(12):
        lo = i * 0.25
        hi = lo + 0.25
        n = sum(1 for r in rows if lo <= r[key] < hi or (i == 11 and r[key] >= lo))
        bins.append({"lo": lo, "hi": hi if i < 11 else None, "count": n})
    return bins


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("banks", type=Path)
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()
    pinned = {
        Path(r["path"]).name: r
        for r in json.loads(MANIFEST.read_text())
        if "/Presets/Data/" in r.get("path", "") and r["path"].endswith(".json")
    }
    owner = json.loads(OWNER.read_text())
    rows = []
    checked = []
    for name, pin in sorted(pinned.items()):
        path = args.banks / name
        data = path.read_bytes()
        digest = hashlib.sha256(data).hexdigest()
        if digest != pin["sha256"]:
            raise SystemExit(f"hash mismatch {name}")
        presets = json.loads(data)
        checked.append({"file": name, "sha256": digest, "presets": len(presets), "bytes": len(data)})
        for item in presets:
            for sr, label in ((48000, "48k"), (44100, "44k")):
                rec = preset_record(name, item, sr)
                rec["rate"] = label
                rec["sr"] = sr
                rows.append(rec)

    def pack(sr_label, factory_only):
        subset = [r for r in rows if r["rate"] == sr_label and (not factory_only or r["bank"] != "User.json")]
        def count(field, limit):
            return sum(1 for r in subset if r[field] <= limit)
        return {
            "presets": len(subset),
            "heard_within_dly_buf": count("heard_bytes", DLY_BUF),
            "armed_within_dly_buf": count("armed_bytes", DLY_BUF),
            "heard_within_send_only": count("heard_bytes", RECLAIM_SEND),
            "armed_within_send_only": count("armed_bytes", RECLAIM_SEND),
            "heard_within_keep_engines": count("heard_bytes", RECLAIM_KEEP_ENGINES),
            "armed_within_keep_engines": count("armed_bytes", RECLAIM_KEEP_ENGINES),
            "heard_within_no_screen": count("heard_bytes", RECLAIM_KEEP_ENGINES_NO_SCREEN),
            "armed_within_no_screen": count("armed_bytes", RECLAIM_KEEP_ENGINES_NO_SCREEN),
            "heard_within_pool_ceiling": count("heard_bytes", POOL_CEILING),
            "armed_within_pool_ceiling": count("armed_bytes", POOL_CEILING),
            "delay_bytes_within_dly_buf": sum(1 for r in subset if r["delay_bytes"] <= DLY_BUF),
            "effective_delay_histogram": histogram(subset, "effective_delay_s"),
            "max_armed_bytes": max(r["armed_bytes"] for r in subset),
            "max_heard_bytes": max(r["heard_bytes"] for r in subset),
            "max_delay_bytes": max(r["delay_bytes"] for r in subset),
        }

    summary = {
        "source": "AudioKit/AudioKitSynthOne@6466a371 Presets/Data, SHA-256 matched to public-source-manifest.json",
        "tempo": "Fresh-kernel load order from S1DSPKernel: delayTime is snapped with nearestTime at 120 BPM while sync is still the default on, then arpRate re-snaps that result at the preset tempo, then tempoSyncToArpRate is stored. Sync off does not restore the pre-snap seconds. Divisions are two bars through a 1/64 triplet. 8, 6, 4 and 3 bars are excluded. Times outside 0.0003628117914..2.5 s are skipped.",
        "line_formula": "Each line is ceil(seconds*sr)+4 floats, the host-proven bit-exact window. Left and right lines are 2*delayTime. The other two are delayTime.",
        "reverb_bytes_48k": revsc_bytes(48000),
        "reverb_bytes_44k": revsc_bytes(44100),
        "widen_bytes_48k": widen_bytes(48000),
        "widen_bytes_44k": widen_bytes(44100),
        "dly_buf": DLY_BUF,
        "pool_bytes": POOL,
        "pool_floor": FLOOR,
        "pool_ceiling": POOL_CEILING,
        "reclaim": {
            "send_delay_dly_buf": {"bytes": DLY_BUF, "owner": "drum/bass send delay, engine.c dly_buf"},
            "undo": {"bytes": UNDO, "owner": "UI undo history, undo.c"},
            "scope": {"bytes": SCOPE, "owner": "engine scope meter, engine.c eng_scope"},
            "ble_midi_out": {"bytes": BLE_OQ, "owner": "Bluetooth MIDI output queue, ble.c"},
            "screen_strip": {"bytes": CANVAS, "owner": "screen strip, gfx.c cv_px"},
            "keep_engines": RECLAIM_KEEP_ENGINES,
            "keep_engines_drop_screen": RECLAIM_KEEP_ENGINES_NO_SCREEN,
            "pool_ceiling_removes_engines_too": POOL_CEILING,
        },
        "files": checked,
        "factory_48k": pack("48k", True),
        "factory_44k": pack("44k", True),
        "with_user_bank_48k": pack("48k", False),
        "owner_archive_not_counted": {
            "sha256": owner.get("archive_sha256"),
            "presets": sum(b["presets"] for b in owner["banks"]),
            "reason": "Private Synth one.zip is not in this checkout. Generated preset-params.f32 must stay uncommitted.",
        },
        "presets": rows,
    }
    args.out.write_text(json.dumps(summary, indent=2) + "\n")
    f48 = summary["factory_48k"]
    print(json.dumps({
        "factory_presets": f48["presets"],
        "heard_in_dly": f48["heard_within_dly_buf"],
        "armed_in_dly": f48["armed_within_dly_buf"],
        "heard_in_ceiling": f48["heard_within_pool_ceiling"],
        "armed_in_ceiling": f48["armed_within_pool_ceiling"],
        "delay_only_in_dly": f48["delay_bytes_within_dly_buf"],
        "histogram": f48["effective_delay_histogram"],
        "max_armed": f48["max_armed_bytes"],
        "max_delay": f48["max_delay_bytes"],
    }, indent=2))


if __name__ == "__main__":
    main()
