#!/usr/bin/env python3
"""Firmware UI visual regression gate (host simulator, exact pixels).

  python3 tests/host/ui_visual_gate/gate.py                 render every canonical state, diff vs golden.png: any diff FAILS
  python3 tests/host/ui_visual_gate/gate.py --accept ID... --who NAME --words "VERBATIM APPROVAL"
                                                            rewrite those golden.png and append the approval note (no other path)
  python3 tests/host/ui_visual_gate/gate.py --conformance   non-blocking: golden vs target (the R22 gap), report only

Tolerance is 0: a state passes only when every decoded RGB pixel of its frame equals
golden.png (goldens are stored re-encoded, optimised, so the comparison is on pixels, not
on PNG bytes); a mismatch reports the changed pixel count and bounding box.
The render is deterministic: fixed simulated clock (X0X_HOST_FIXED_CLOCK=1), fixed load
(X0X_HOST_LOAD=0), fresh simulated flash per state, no seed or wall time reaches ui.c.
Coverage: the state catalogue is re-enumerated from the firmware each run (catalog.py);
a reachable page / list row / help card not in docs/ui/CANONICAL/scenes.json FAILS.
"""
import argparse, datetime, hashlib, json, os, shutil, subprocess, sys
from pathlib import Path
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import catalog as cat
ROOT = cat.ROOT
CAN = ROOT / 'docs/ui/CANONICAL'
OUT = ROOT / 'build/ui-visual-gate'

def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def render(scene, out):
    d = out / scene['id']
    if d.exists(): shutil.rmtree(d)
    cat.run(scene['commands'] + '\nshot frame.png', d)
    return d / 'frame.png'

def pixels(p):
    from PIL import Image
    im = Image.open(p).convert('RGB')
    return im.size, im.tobytes()

def store(frame, golden):
    from PIL import Image
    golden.parent.mkdir(parents=True, exist_ok=True)
    Image.open(frame).convert('RGB').save(golden, optimize=True)

def pixels_changed(a, b):
    from PIL import Image, ImageChops
    ia, ib = Image.open(a).convert('RGB'), Image.open(b).convert('RGB')
    if ia.size != ib.size: return ia.size[0] * ia.size[1], None
    diff = ImageChops.difference(ia, ib)
    n = sum(1 for p in diff.getdata() if p != (0, 0, 0))
    return n, diff.getbbox()

def load_scenes():
    return json.loads((CAN / 'scenes.json').read_text())

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--accept', nargs='*')
    ap.add_argument('--who'); ap.add_argument('--words')
    ap.add_argument('--conformance', action='store_true')
    ap.add_argument('--bootstrap', action='store_true', help=argparse.SUPPRESS)
    a = ap.parse_args()
    if not (ROOT / 'build/host/x0x_host').exists():
        subprocess.run(['sh', 'host/build_host.sh'], cwd=ROOT, check=True)
    if a.conformance:
        return conformance()
    scenes = load_scenes()
    OUT.mkdir(parents=True, exist_ok=True)
    if a.accept is not None:
        return accept(scenes, a)
    # 1) coverage
    fail = 0
    now = [dict(id=s['id'], commands=s['commands']) for s in cat.catalog()]
    have = {s['id']: s['commands'] for s in scenes}
    for s in now:
        if s['id'] not in have:
            print(f"FAIL coverage: new reachable state {s['id']} has no canonical entry"); fail += 1
        elif have[s['id']] != s['commands']:
            print(f"FAIL coverage: {s['id']}: the way to reach it changed (catalogue differs)"); fail += 1
    gone = set(have) - {s['id'] for s in now}
    for g in sorted(gone):
        print(f"FAIL coverage: canonical state {g} is no longer enumerated"); fail += 1
    # 2) render twice-free exact diff
    ok = 0
    from concurrent.futures import ThreadPoolExecutor
    jobs = int(os.environ.get('X0X_UI_GATE_JOBS', '8'))
    with ThreadPoolExecutor(jobs) as ex:
        frames = list(ex.map(lambda s: render(s, OUT / 'render'), scenes))
    for s, f in zip(scenes, frames):
        g = CAN / s['id'] / 'golden.png'
        if not g.exists():
            print(f"FAIL {s['id']}: no golden.png"); fail += 1; continue
        if pixels(f) == pixels(g):
            ok += 1; continue
        n, box = pixels_changed(f, g)
        print(f"FAIL {s['id']}: {n} pixel(s) differ from golden.png (bbox {box}); frame {f.relative_to(ROOT)}")
        fail += 1
    print(f"ui-visual-gate: {len(scenes)} states, {ok} golden-pass, {fail} fail (tolerance 0)")
    return 1 if fail else 0

def accept(scenes, a):
    if not a.who or not a.words or not a.accept:
        print('--accept needs state ids, --who and --words (the verbatim approval)'); return 2
    byid = {s['id']: s for s in scenes}
    ids = list(byid) if a.accept == ['ALL'] else a.accept
    changed = []
    for i in ids:
        if i not in byid: print(f'unknown state {i}'); return 2
        f = render(byid[i], OUT / 'render')
        g = CAN / i / 'golden.png'
        old = sha(g) if g.exists() else '-'
        if not g.exists() or pixels(f) != pixels(g):
            store(f, g); changed.append((i, old, sha(g)))
    when = datetime.datetime.now().astimezone().isoformat(timespec='minutes')
    with open(CAN / 'APPROVALS.tsv', 'a') as fh:
        for i, old, new in changed:
            fh.write(f"{when}\t{a.who}\t{i}\t{old}\t{new}\t{a.words}\n")
    with open(CAN / 'CANONICAL.md', 'a') as fh:
        fh.write(f"\n- {when} — approved by {a.who}: \"{a.words}\" — golden updated: "
                 + ((', '.join(f'`{i}` {o[:8]}→{n[:8]}' for i, o, n in changed) or 'none changed') if len(changed) <= 20
                    else f'{len(changed)} states (each listed with old/new sha256 in APPROVALS.tsv)') + "\n")
    print(f'accepted {len(changed)} golden change(s); approval note written')
    return 0

def conformance():
    rows = json.loads((CAN / 'index.json').read_text())
    m = d = 0
    lines = ['# Conformance gap: golden (firmware today) vs target (R22 + approved deltas)', '',
             'Non-blocking. MATCHES-TARGET only when the entry records it; computed pixel gap shown for every targeted state.', '',
             '| state | status | target | changed px vs target | owner |', '|---|---|---|---:|---|']
    for r in rows:
        t = CAN / r['id'] / 'target.png'
        gap = '-'
        if t.exists():
            n, _ = pixels_changed(CAN / r['id'] / 'golden.png', t); gap = str(n)
        if r['status'] == 'MATCHES-TARGET': m += 1
        else: d += 1
        lines.append(f"| {r['id']} | {r['status']} | {r.get('target_src','-')} | {gap} | {r.get('owner','-')} |")
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'conformance.md').write_text('\n'.join(lines) + '\n')
    print(f'conformance (non-blocking): {m} MATCHES-TARGET, {d} KNOWN-DEVIATION; {OUT / "conformance.md"}')
    return 0

if __name__ == '__main__':
    sys.exit(main())
