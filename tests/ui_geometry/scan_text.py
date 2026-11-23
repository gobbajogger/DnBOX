#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""List text proven visible in reached frames and flag known prose for F7 review."""
import argparse
from collections import defaultdict
import json
from pathlib import Path
import re

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
    "PICKUP: MOVE UP TO TARGET",
    "PICKUP: MOVE DOWN TO TARGET",
    "TEACH EACH BUTTON AND KNOB",
    "IT CRASHED TWICE",
    "WHILE STARTING.",
    "NO SOUND. USB IS ON:",
    "REINSTALL FROM THE",
    "WEB INSTALLER.",
    "THE SECOND CORE CRASHED",
    "RESTARTED AFTER A CRASH",
    "PAD 1 TOP-LEFT?",
    "THIS SCREEN AS A LIST",
    "THE PART'S SOUND",
    "IN A LIST: RUN THE ACTION",
    "EMPTY: WHITE KEYS ADD BARS",
    "EMPTY   WHITE KEY: ADD BAR",
}

# L14: outside the on-demand hold-a-button help screens, no visible caption may
# be a question or an instruction ("CONTROL: ACTION", "TAP: ...", "+ KNOB: ...").
INSTRUCTION = re.compile(r"\?|^\+ |\b(TAP|HOLD|PRESS|TURN)\b.*[:+]|"
                         r"\b(KEYS?|KNOBS?|PADS?|STEP|BUTTONS?|SEQ|SCALE|SEL|HOME|PATCH|GLOBE)\b[^:]*: ")


def is_instruction(caption):
    return bool(INSTRUCTION.search(caption))


# UI-SOURCE-OF-TRUTH: no cryptic footer/navigation glyph strings ("<> ↑↓ PARA →").
CRYPTIC = re.compile(r"<>|[\u2190-\u2193]|\bPARA\b")


def truncated_words(screens):
    """Words clipped by a character budget (RELEAS for RELEASE, MARAC for MARACAS):
    a word of five or more letters that is another visible word minus its last one
    or two letters. Plurals and past tenses are real words, not clipping."""
    words = defaultdict(set)
    for screen, captions in screens.items():
        if screen.startswith("help-"):
            continue
        for caption in captions:
            for word in re.findall(r"[A-Za-z]+", caption):
                words[word.upper()].add(screen)
    found = {}
    for word in words:
        if len(word) < 5:
            continue
        for longer in words:
            tail = longer[len(word):]
            if longer.startswith(word) and 1 <= len(tail) <= 2 and tail not in ("S", "D", "ED", "ES", "ER", "R"):
                found[word] = sorted(words[word])
    return found


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
    for screen, captions in screens.items():
        if screen.startswith("help-"):
            continue
        for caption in captions:
            if is_instruction(caption) and screen not in findings.get(caption, []):
                findings.setdefault(caption, []).append(screen)
    for screen, captions in screens.items():
        if screen.startswith("help-"):
            continue
        for caption in captions:
            if CRYPTIC.search(caption) and screen not in findings.get(caption, []):
                findings.setdefault(caption, []).append(screen)
    for word, ids in truncated_words(screens).items():
        findings.setdefault("TRUNCATED " + word, []).extend(ids)
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
