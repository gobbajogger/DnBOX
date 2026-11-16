#!/usr/bin/env python3
"""(Re)build docs/ui/CANONICAL/index.json, each entry's target.png and CANONICAL.md.

Never touches golden.png (only gate.py --accept does). Inputs on the box, not in git:
  R22 gallery renders   /workspace/dnbox-mockups/r23/_r22png/<tag>.png  (480 px, from Dot's R22 SVGs)
  R22 manifest          /workspace/dnbox-mockups/from-dot/R22-gallery/manifest.json
  python3 tests/host/ui_visual_gate/build_canonical.py [--r22png DIR] [--manifest FILE]
"""
import argparse, json, re
from pathlib import Path
import numpy as np
from PIL import Image
ROOT = Path(__file__).resolve().parents[3]
CAN = ROOT / 'docs/ui/CANONICAL'
FAR = 'excluded: future-engine study / known-quirk evidence'
EXCLUDE_GROUPS = {'Future-engine control studies', 'Known source quirk evidence'}

def explicit(sid):
    """Hand-mapped targets: the R22 presentation view for that state (newest approved)."""
    m = re.match(r'(909|808)-para-00(?:-voice-(\d\d))?$', sid)
    if m: return f'{m.group(1)}-voice-{m.group(2) or "00"}'
    m = re.match(r'(?:909|808|break|chicago|miami)-mix-(\d\d)$', sid)
    if m: return {'00': 'mix-parts-live', '01': 'mix-909-00', '02': 'mix-808-00', '03': 'mix-master', '04': 'mix-wave'}[m.group(1)]
    m = re.match(r'(?:909|808|break|chicago|miami)-home-00$', sid)
    if m: return 'home-01-16'
    m = re.match(r'(?:909|808|break|chicago|miami)-fx-(\d\d)$', sid)
    if m: return {'00': 'fx-reverb-direct', '01': 'fx-delay-direct'}[m.group(1)]
    return None

OWNER = {
    'battery': 'R024 battery-% lane (branch not created yet; measured % or bars only)',
    'footer': 'R22 no-explainer delta — lead/glyph-aa-ease-20261116 (UI polish lane)',
    'glyphs': 'all-glyphs-live delta — lead/glyph-aa-ease-20261116',
    'pan': 'pan L50..C..R50 delta — lead/pan-clip-50-20261116 (1d02a091)',
    'drumpara': 'R22 909/808 voice page — lead/drum-para-r22-20261116',
    'body': 'R22 body/layout gap vs target — owner per SCREEN-INVENTORY row (lead UI polish batches 2–4)',
}

def load(p, size=240):
    im = Image.open(p).convert('RGB')
    if im.size != (size, size): im = im.resize((size, size), Image.LANCZOS)
    return im

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--r22png', default='/workspace/dnbox-mockups/r23/_r22png')
    ap.add_argument('--manifest', default='/workspace/dnbox-mockups/from-dot/R22-gallery/manifest.json')
    a = ap.parse_args()
    man = {x['tag']: x for x in json.loads(Path(a.manifest).read_text())}
    cands = [t for t, x in man.items() if x['group'] not in EXCLUDE_GROUPS and (Path(a.r22png) / f'{t}.png').exists()]
    arr = {t: np.asarray(load(Path(a.r22png) / f'{t}.png'), dtype=np.int16) for t in cands}
    small = {t: v[::4, ::4].astype(np.float32) for t, v in arr.items()}   # 60x60 for the nearest-view search
    scenes = json.loads((CAN / 'scenes.json').read_text())
    rows = []
    for s in scenes:
        sid = s['id']; g = CAN / sid / 'golden.png'
        ga = np.asarray(Image.open(g).convert('RGB'), dtype=np.int16)
        base = re.sub(r'^theme-[a-z]+-', '', sid)
        tag, how = explicit(sid), 'hand-mapped R22 presentation view'
        if tag is None:
            touch = sid.endswith('-touch')
            pool = [t for t in cands if t.startswith('touch-') == touch] or cands
            gs = ga[::4, ::4].astype(np.float32)
            dist = {t: float(np.abs(small[t] - gs).mean()) for t in pool}
            tag = min(dist, key=dist.get)
            how = f'auto-mapped: nearest R22 gallery view by mean |ΔRGB| ({dist[tag]:.1f}); review in index.json'
        ta = arr[tag]
        gap = int((np.abs(ta - ga).max(axis=2) > 0).sum())
        dev = ['battery']                      # no battery % in draw_header at 94197fc8 (only GLO > SYSTEM > BATTERY)
        foot = ga[229:240, :, :]
        if int((foot.max(axis=2) > 90).sum()) > 20: dev.append('footer')
        dev.append('glyphs')                   # SCREEN-INVENTORY: static stamps on every page family; no eased AA renderer at base
        if re.search(r'-mix-0[012]', sid) or re.search(r'(909|808)-para', sid): dev.append('pan')
        if re.match(r'(theme-[a-z]+-p[01]-para|(909|808)-para)', sid): dev.append('drumpara')
        if gap: dev.append('body')
        status = 'MATCHES-TARGET' if not dev else 'KNOWN-DEVIATION'
        tp = CAN / sid / 'target.png'
        load(Path(a.r22png) / f'{tag}.png').save(tp, optimize=True)
        x = man[tag]
        rows.append(dict(id=sid, status=status, target_src=f'from-dot/R22-gallery/views/{tag}.svg', target_tag=tag,
                         target_group=x['group'], target_kind=x['kind'], target_svg_sha256=x['sha256'], mapping=how,
                         pixels_differ_vs_target=gap, deviations=dev, owner='; '.join(OWNER[d] for d in dev)))
    (CAN / 'index.json').write_text(json.dumps(rows, indent=1) + '\n')
    write_md(rows, scenes)
    n = sum(r['status'] == 'MATCHES-TARGET' for r in rows)
    print(f'{len(rows)} entries: {n} MATCHES-TARGET, {len(rows) - n} KNOWN-DEVIATION')

def write_md(rows, scenes):
    md = CAN / 'CANONICAL.md'
    old = md.read_text() if md.exists() else ''
    log = old[old.index('## Golden approval log'):] if '## Golden approval log' in old else '## Golden approval log\n'
    from collections import Counter
    c = Counter(d for r in rows for d in r['deviations'])
    h = [
        '# Canonical firmware screens (the one reference set)', '',
        'This directory is the **only** canonical reference for what each firmware screen and state looks like.',
        'Every other capture or mockup is NON-CANONICAL (see `NON-CANONICAL.md`). Nothing was deleted.', '',
        '## Sources and approval', '',
        '- **target.png** — the newest approved design: Dot\'s **R22 gallery** (`/workspace/dnbox-mockups/from-dot/R22-gallery/views/*.svg`, '
        'rendered 480 px in `r23/_r22png/`, downscaled to the 240×240 panel). Approved: the user picked R22 for all glyphs on 2026-11-16. '
        'The user\'s approved deltas apply on top of every target: pan L50..C..R50 clip, per-voice pan, no explainer text, '
        'all glyphs live and animated, battery % in the top indicator. A target that already shows a delta\'s opposite (e.g. an R22 '
        'explainer footer) is overridden by the delta.',
        '- **golden.png** — the deterministic host render of the firmware at the branch base `94197fc8` '
        '(`tests/host/ui_visual_gate/gate.py`, fixed clock, load 0). The visual gate diffs every build against it with tolerance 0.',
        '- **status** — `MATCHES-TARGET` or `KNOWN-DEVIATION` (what differs, and the owning fix branch). Report: `gate.py --conformance`.', '',
        'Target mapping: hand-mapped where an R22 presentation view exists for the state (909/808 voice pages, MIX parts/909/808/MASTER/WAVE, '
        'HOME, FX); otherwise the nearest R22 gallery view (recorded with its distance in `index.json`, to review).', '',
        '## Coverage', '',
        f'{len(rows)} states, enumerated from the firmware by `tests/host/ui_visual_gate/catalog.py` through real key/encoder input: '
        'every page of SEQ / sound (EDIT) / FX / MIX / HOME / TB-3PO for each of the 5 parts; every drum voice on SEQ and sound pages; '
        'a knob-touch readout and a held step per page family; every row of every SEL list and GLOBAL section; every button\'s help card; '
        'SONG; all 6 themes on each top-level view; transport play/REC; and every `shot` of the interaction scenarios '
        '(tour, song, help, sound matrices, page split, save feedback, groove, patterns, clock source, PERF, guard, jam, showcase, stereo), '
        'which reach the dialogs (CLEAR / RESET / LOAD / COPY / RESET SOUND), cue/chain, recording, motion, toasts and PERF results. '
        'A newly reachable page, list row or help card that is not here fails the gate (coverage check).', '',
        'Not rendered by the FM-1 host sim (counted as **FAIL / not covered**): the SMK-37-only clip grid, WAITING FOR LOOPY PRO and pad-calibration '
        'screens (`BOARD_SMK37` in `draw_main`), and the SMC controller screen (`smc_mixer.c`, covered by its own framebuffer tests, not this gate).', '',
        '## Conformance at base (golden vs target)', '',
        f'MATCHES-TARGET: {sum(r["status"]=="MATCHES-TARGET" for r in rows)}; KNOWN-DEVIATION: {sum(r["status"]!="MATCHES-TARGET" for r in rows)}.', '',
        '| deviation | states | owner |', '|---|---:|---|',
    ] + [f'| {d} | {c[d]} | {OWNER[d]} |' for d in OWNER if c[d]] + [
        '', '## Entries', '',
        '| state | status | target (R22) | px ≠ target | deviations |', '|---|---|---|---:|---|',
    ] + [f'| [`{r["id"]}`]({r["id"]}/) | {r["status"]} | `{r["target_tag"]}` | {r["pixels_differ_vs_target"]} | {", ".join(r["deviations"])} |' for r in rows] + ['', '']
    md.write_text('\n'.join(h) + log)

if __name__ == '__main__':
    main()
