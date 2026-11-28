#!/usr/bin/env bash
set -euo pipefail
p=$(cd "$(dirname "$0")" && pwd)
src=$(realpath "${1:?clean isolated exact e00 checkout}")
model=${2:?pro or elite}
variant=${3:?control, early, peer, peer-full, trace, fixed, lazy, or phase}
case "$model" in pro|elite) ;; *) exit 2;; esac
cd "$src"
test "$(git rev-parse HEAD)" = e00f689dcab38f396dc23325e0f8eef515925489
test -z "$(git status --porcelain --untracked-files=no)"
for patch in viewport82-e00 staged82-lz e00-82-cold-app-fit e00-82-extra-cold e00-82-project-format e00-chicago-fast32-minimax; do
    git apply "$p/../display/$patch.patch"
done
git apply "$p/../worker-typed-s1-only.patch" "$p/../808-low-tanh.patch" "$p/../909-phase-fold.patch"
git apply "$p/overlap-both-kits.patch"
case "$variant" in
  control) flags='-DX0X_CORE_PLAN=3 -DDNBOX_NO_B_OVERLAP=1';;
  early) flags='-DX0X_CORE_PLAN=3';;
  peer|peer-full) git apply "$p/peer-s1-optional.patch"; flags='-DX0X_CORE_PLAN=8 -DDNBOX_B_PEER_TRIAL=1'
    if [[ $variant == peer-full ]]; then flags+=' -DDNBOX_B_PEER_FULL808=1'; fi;;
  trace) git apply "$p/peer-s1-optional.patch" "$p/trace.patch"; flags='-DX0X_CORE_PLAN=3';;
  fixed|lazy|phase) git apply "$p/fixed-smoothing.patch"; flags='-DX0X_CORE_PLAN=3 -DDNBOX_S1_SKIP_FIXED=1'
    if [[ $variant != fixed ]]; then git apply "$p/lazy-peer.patch"; flags+=' -DDNBOX_S1_LAZY_PEER=1'; fi
    [[ $variant != phase ]] || { git apply "$p/osc-phase-cache.patch"; flags+=' -DDNBOX_S1_PHASE_CACHE=1'; };;
  *) exit 2;;
esac
python3 - <<'PY'
from pathlib import Path
p=Path("tools/build.py")
s=p.read_text()
t='''            if S1C == "1" and Path(u).stem in S1C_OZ_UNITS:
                uf = ["-Oz" if flag == "-Os" else flag for flag in uf]'''
assert t in s
s=s.replace(t,t+'''
            if S1C == "1" and Path(u).stem in {"s1c","s1u"}:
                uf = ["-O3" if flag in ("-Os","-Oz","-O2") else flag for flag in uf]''')
p.write_text(s)
PY
MODEL="$model" X0X_UI_SMALL_CANVAS=1 X0X_DEV_PADCAL=0 X0X_GEN_CACHE=0 OVERLAP_CFLAGS="$flags" python3 -c 'import os; import tools.build as b; b.CFLAGS.extend([*os.environ["OVERLAP_CFLAGS"].split(),"-DDNBOX_S1_WORK_SPLIT=1","-DDNBOX_S1_SPLIT_START=1","-DS1C_SPLIT_SPAN=112u"]); b.ORIGINAL_SYNTH_O2_UNITS.update(["s1c","s1u"]); raise SystemExit(b.main())' --board smk37 --nightly-version 0.0.0 --git-sha e00f689dcab38f396dc23325e0f8eef515925489
