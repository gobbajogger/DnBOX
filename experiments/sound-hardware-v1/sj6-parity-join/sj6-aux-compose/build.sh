#!/bin/sh
set -eu
base="$PWD/firmware-fit-next/tree/firmware/src"
donor="$PWD/sj6-fold32-recovery/exact-donor/firmware/src/app"
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
ln -s "$base/dsp" "$T/dsp"
ln -s "$base/x0x_cold.h" "$T/x0x_cold.h"
python sj6-runtime-source-mix/wire.py "$donor" "$T/app" "$base/app"
baseflags="-std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1"
inc="-I$base/../../experiments/original_synth -I$base/app -I$base/dsp/synthone -I$base/dsp"
cp "$donor/original_runtime.c" "$T/app/base.c"
cc $baseflags $inc -E -P "$T/app/base.c" > "$T/off-base.i"
cc $baseflags $inc -E -P "$T/app/original_runtime.c" > "$T/off-guard.i"
cmp "$T/off-base.i" "$T/off-guard.i"
cc -std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SJ6_SOURCE_MIX_WITNESS=1 -ffunction-sections -fdata-sections -O2 -ffp-contract=off -fPIC -I"$base/../../experiments/original_synth" -I"$base/app" -I"$base/dsp/synthone" -I"$base/dsp" -c "$T/app/original_runtime.c" -o "$T/runtime.o"
cat >>"$T/app/original_runtime.c" <<'C'
int sj6_probe(const sj6_source_block *b,float *l,float *r,unsigned n,float master)
{
    original_runtime_part p={0};
    original_runtime_workspace w;
    p.kind=ORIGINAL_SOUND_JUNO;
    p.status=ORIGINAL_RUNTIME_OK;
    p.dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
#if !X0X_SUPERJ6_BUILTIN_INIT
    p.dsp.juno.synth.typed=1;
#endif
    p.dsp.juno.synth.master_gain=master;
    p.sj6_source_pending=b;
    return render_backend(&p,&w,l,r,n);
}
C
flags="-std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SJ6_SOURCE_MIX_WITNESS=1 -O2 -ffp-contract=off -fPIC"
cc $flags -I"$base/../../experiments/original_synth" -I"$base/app" -I"$base/dsp/synthone" -I"$base/dsp" -c "$T/app/original_runtime.c" -o "$T/runtime.o"
cc $flags -I"$base/dsp" -c "$base/dsp/juno6.c" -o "$T/juno.o"
cc -shared "$T/runtime.o" "$T/juno.o" -lm -o "$T/owner.so"
ORIGINAL_SO="$T/owner.so" PYTHONPATH=/tmp/sj6-room-next-py:$PWD/ab-effects-readonly/sj6-fullbench/bench python sj6-aux-compose/check.py "$@"
