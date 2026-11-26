#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")";pwd); p=${1:-/tmp/phaser-warm-own.Y0XNbQ/tree}; s="$p/firmware/src"; o="$p/experiments/original_synth"
DIR=$(mktemp -d "$d/check-out.XXXX")
trap 'rm -r "$DIR"' EXIT
export TMPDIR="$DIR"
(cd "$p"; sha256sum -c "$d/owner-fixture.pins")
python3 - "$s/app/engine.c" >"$DIR/exact-glide.inc" <<'PY'
import hashlib,sys
s=open(sys.argv[1],'rb').read()
assert hashlib.sha256(s).hexdigest()=="92a7d2f68d027c40306a27bf954ea7f5a736e14cc8d169a97ef9b30e56cb4f54"
s=s.decode()
print(s[s.index('typedef struct { float cur, tgt; } glide_t;'):s.index('/* each part\'s gains:')])
a=s.index('__attribute__((noinline)) static void scale_glide(')
print(s[a:s.index('\n}',a)+3])
PY
cc -std=gnu11 -O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -pthread -DX0X_HOST -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$s/dsp" -I"$s" -I"$p/build/smk37-elite/gen" -I"$o" -I"$DIR" "$d/owner-fixture.c" "$s/dsp/drum909.c" "$s/dsp/drum808.c" "$s/dsp/x0x_param.c" "$o/chicago_mp4.c" -Wl,--gc-sections -lm -o "$DIR/owner-fixture"
"$DIR/owner-fixture"
