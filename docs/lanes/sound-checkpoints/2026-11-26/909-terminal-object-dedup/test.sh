#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")"&&pwd); p=${1:-/tmp/phaser-warm-own.Y0XNbQ/tree}; s="$p/firmware/src"
b=$(mktemp -d "$d/test.XXXX"); trap 'rm -rf "$b"' EXIT; export TMPDIR="$b"
for f in firmware/src/{app/engine.c,app/original_runtime.h,dsp/drum909.c,dsp/drum909.h};do mkdir -p "$b/$(dirname "$f")"; cp "$p/$f" "$b/$f";done
(cd "$b"; patch -p1 --fuzz=0 <"${TERMINAL_PATCH:-$d/diagnostic.patch}")
python3 - "$b/firmware/src/app/engine.c" >"$b/scheduler.inc" <<'PY'
import sys
t=open(sys.argv[1]).read()
def f(sig):
 i=t.index(sig+'\n{');return t[i:t.index('\n}',i)+3]
for name in ('static void terminal_check_halted(void)','static void core2_jobs(uint32_t arg)','static void c1_take_over(uint32_t s)'):
 print(f(name))
PY
python3 - "$b/firmware/src/dsp/drum909.c" <<'PYEDIT'
from pathlib import Path
import sys
p=Path(sys.argv[1]);t=p.read_text()
old='    plat_cpu2_sync();\n    f->ready=token|1u;'
assert t.count(old)==1
p.write_text('static void test_pre_publish(void);\n'+t.replace(old,'    test_pre_publish();\n'+old))
PYEDIT
cat >"$b/drums.c" <<DOG
#include "$b/firmware/src/dsp/drum909.c"
#include "drum808.h"
#include "$d/test.inc"
DOG
cc -std=gnu11 -O2 -ffp-contract=off -pthread -ffunction-sections -fdata-sections -DX0X_DIAG_909_TERMINAL_GROUP=1 -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$b" -I"$s/dsp" -I"$p/build/smk37-elite/gen" "$b/drums.c" "$s/dsp/drum808.c" "$s/dsp/x0x_param.c" -Wl,--gc-sections -lm -o "$b/test"
timeout 20 "$b/test"
