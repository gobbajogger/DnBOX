#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""List text proven visible in reached frames and flag known prose for F7 review."""
import argparse
from collections import defaultdict
import json
from pathlib import Path

# Confirmed instructional sentences from the existing renderers. Keep a complete
# register of observed strings, including new words requiring editorial review.
INSTRUCTIONAL = {
    "THE KEYS PLAY AND WRITE NOTES",
    "HOLDING A STEP: ITS OCTAVE",
    "THE EFFECTS: REVERB, DELAY,",
    "AGAIN: THE NEXT PAGE",
    "THE PATTERN: ALL FIVE PARTS",
    "THE MIXER AND THE MASTER",
    "CHOOSE PASTE TARGET",
    "THIS SCREEN AS A LIST",
    "THE PART'S SOUND",
    "IN A LIST: RUN THE ACTION",
    "EMPTY: WHITE KEYS ADD BARS",
}


def scan(log):
    screens = defaultdict(set)
    traced = defaultdict(int)
    audited = {}
    complete = None
    for line in log:
        if line.startswith("UI_TEXT\t"):
            _, screen, _x, _y, caption = line.rstrip("\n").split("\t", 4)
            screens[screen].add(caption)
            traced[screen] += 1
        elif line.startswith("AUDIT "):
            row = json.loads(line[6:])
            if row["kind"] == "complete":
                complete = row["screens"]
            elif row["id"] in audited or row["findings"] or row["texts"] < 1:
                raise ValueError(f"Incomplete or duplicate raster audit for {row['id']}")
            else:
                audited[row["id"]] = row["texts"]
    if not screens or set(screens) != set(audited) or len(audited) != complete or dict(traced) != audited:
        raise ValueError("Missing final-frame text trace or incomplete geometry walk")
    findings = {caption: sorted(screen for screen in screens if caption in screens[screen])
                for caption in sorted(INSTRUCTIONAL)}
    for suffix in (": THIS TRACK", ": THE PATTERN", ": THE SOUND"):
        for screen, captions in screens.items():
            for caption in captions:
                if caption.endswith(suffix):
                    findings.setdefault(caption, []).append(screen)
    return {"audited_screens": len(audited), "distinct_strings": len(set.union(*screens.values())),
            "known_instructional_copy": {s: ids for s, ids in findings.items() if ids},
            "screen_text": {s: sorted(captions) for s, captions in sorted(screens.items())}}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("render_log", type=Path, help="positive geometry run with FWUI_TEXT_TRACE=1")
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    with args.render_log.open() as source:
        result = scan(source)
    args.report.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    for caption, screens in result["known_instructional_copy"].items():
        print(f"{len(screens)} screens display {caption!r}; first frame {screens[0]}")
    print(f"{result['audited_screens']} reached screens, {result['distinct_strings']} distinct strings")
    raise SystemExit(bool(result["known_instructional_copy"]))
