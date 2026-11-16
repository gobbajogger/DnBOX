#!/usr/bin/env python3
"""Count Synth One presets in this checkout whose delayTime exceeds 0.264 s.

Bank JSON is not committed. This script reads only files in the repo. It does
not fetch the private owner archive or the public AudioKit preset bodies.
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CAP = 0.26435416666666667
EXP = ROOT / "experiments/synthone-port-feasibility-20261114"
PUBLIC = EXP / "public-source-manifest.json"
OWNER = EXP / "owner-corpus-manifest.json"
PROBE = EXP / "probe/voice-probe-inputs.json"
HEADER = EXP / "probe/portable_interface.hpp"


def defaults_from_header():
    text = HEADER.read_text()
    start = text.index("static const float defaults[150]={")
    body = text[start:].split("{", 1)[1].split("}", 1)[0]
    vals = [float(x.strip().rstrip("f")) for x in body.split(",") if x.strip()]
    names = ["delayTime", "arpRate", "tempoSyncToArpRate"]
    # Indices from the enum in the same header.
    enum = text[text.index("enum S1Parameter"):text.index("};", text.index("enum S1Parameter"))]
    index = {}
    n = 0
    for line in enum.splitlines():
        line = line.split("//", 1)[0].strip().strip(",")
        if not line or line.startswith("enum"):
            continue
        if "=" in line:
            key, raw = [p.strip() for p in line.split("=", 1)]
            n = int(raw)
            index[key] = n
            n += 1
        else:
            index[line] = n
            n += 1
    return {name: vals[index[name]] for name in names}


def public_banks():
    rows = json.loads(PUBLIC.read_text())
    out = []
    for row in rows:
        path = row.get("path", "")
        if "/Presets/Data/" in path and path.endswith(".json"):
            out.append({
                "kind": "public-factory-pin",
                "path": path,
                "bytes": row.get("size"),
                "sha256": row.get("sha256"),
                "commit": row.get("commit"),
            })
    return out


def owner_banks():
    data = json.loads(OWNER.read_text())
    return {
        "archive_sha256": data.get("archive_sha256"),
        "banks": data.get("banks", []),
    }


def find_body(name):
    hits = list(ROOT.rglob(name))
    return [str(p.relative_to(ROOT)) for p in hits if p.is_file()]


def main():
    defs = defaults_from_header()
    public = public_banks()
    owner = owner_banks()
    probe = json.loads(PROBE.read_text())
    readable = []
    missing = []
    for row in public:
        name = Path(row["path"]).name
        hits = find_body(name)
        entry = dict(row, local_hits=hits)
        (readable if hits else missing).append(entry)
    owner_missing = []
    for row in owner["banks"]:
        name = Path(row["archive_path"]).name
        hits = find_body(name)
        entry = dict(row, local_hits=hits)
        (readable if hits else owner_missing).append(entry)
    above = 0
    total_readable = 0
    hist = {f"{i * 0.25:.2f}-{(i + 1) * 0.25:.2f}": 0 for i in range(10)}
    for row in readable:
        for hit in row["local_hits"]:
            presets = json.loads((ROOT / hit).read_text())
            if isinstance(presets, dict):
                presets = presets.get("presets", [])
            for preset in presets:
                if "delayTime" not in preset:
                    continue
                total_readable += 1
                t = float(preset["delayTime"])
                if t > CAP:
                    above += 1
                bin_i = min(int(t / 0.25), 9)
                key = f"{bin_i * 0.25:.2f}-{(bin_i + 1) * 0.25:.2f}"
                hist[key] += 1
    report = {
        "cap_seconds": CAP,
        "tempo_assumption": (
            "The extracted FX graph reads delayTime as seconds and does not "
            "rewrite it from tempo. In portable_interface.hpp the init default "
            "has tempoSyncToArpRate on and arpRate at 120 BPM. The kernel formula "
            "that would turn that switch into a delay time is not in this checkout, "
            "so no synced effective time was computed. A synced preset would use "
            "its own arpRate when that field is present, otherwise this 120 BPM default."
        ),
        "in_repo_init_default": defs,
        "init_default_above_cap": defs["delayTime"] > CAP,
        "readable_preset_files": len(readable),
        "readable_presets_with_delayTime": total_readable,
        "above_cap": above,
        "histogram_seconds": hist if total_readable else None,
        "public_factory_files_pinned": len(public),
        "public_factory_files_missing": [
            {"path": r["path"], "bytes": r["bytes"], "sha256": r["sha256"]} for r in missing
        ],
        "owner_archive_sha256": owner["archive_sha256"],
        "owner_banks_recorded": [
            {"path": r["archive_path"], "presets": r["presets"], "bytes": r["bytes"]}
            for r in owner["banks"]
        ],
        "owner_banks_missing_bodies": [r["archive_path"] for r in owner_missing],
        "owner_probe_record_count": probe.get("records"),
        "owner_probe_scope": probe.get("scope"),
    }
    json.dump(report, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
