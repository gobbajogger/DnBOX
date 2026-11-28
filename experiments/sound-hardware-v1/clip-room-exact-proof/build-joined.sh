#!/usr/bin/env bash
set -euo pipefail
proof=$(cd "$(dirname "$0")" && pwd)
src=$(realpath "${1:?an isolated checkout at e00f689d}")
m=${2:?pro or elite}
cd "$src"
test "$(git rev-parse HEAD)" = e00f689dcab38f396dc23325e0f8eef515925489
test -z "$(git status --porcelain --untracked-files=no)"
for patch in viewport82-e00 staged82-lz e00-82-cold-app-fit e00-82-extra-cold e00-82-project-format e00-chicago-fast32-minimax; do
   git apply "$proof/display/$patch.patch"
done
git apply "$proof/worker-typed-s1-only.patch"
if [[ ${DUAL_KIT_TRIAL:-0} == 1 ]];then
   git apply "$proof/808-low-tanh.patch" "$proof/909-phase-fold.patch"
fi
python3 - <<'PY'
from pathlib import Path
p=Path('tools/build.py')
s=p.read_text()
t='''            if S1C == "1" and Path(u).stem in S1C_OZ_UNITS:
                uf = ["-Oz" if flag == "-Os" else flag for flag in uf]'''
assert t in s
s=s.replace(t,t+'''
            if S1C == "1" and Path(u).stem in {"s1c","s1u"}:
                uf = ["-O3" if flag in ("-Os","-Oz","-O2") else flag for flag in uf]''')
p.write_text(s)
PY
MODEL="$m" X0X_UI_SMALL_CANVAS=1 X0X_DEV_PADCAL=0 X0X_GEN_CACHE=0 python3 -c 'import tools.build as b; b.CFLAGS.extend(["-DDNBOX_S1_WORK_SPLIT=1","-DDNBOX_S1_SPLIT_START=1","-DS1C_SPLIT_SPAN=112u"]); b.ORIGINAL_SYNTH_O2_UNITS.update(["s1c","s1u"]); raise SystemExit(b.main())' --board smk37 --nightly-version 0.0.0 --git-sha e00f689dcab38f396dc23325e0f8eef515925489
