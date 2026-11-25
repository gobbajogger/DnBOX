#!/bin/bash
set -e
clang="$(find prereqs -path '*/pi32v2/bin/clang' -print -quit)"
base="$(dirname "$(dirname "$clang")")"
target="recovery/source/forward-tree/firmware"
for z in 0; do
 "$clang" -target pi32v2 -mcpu=r3 -mfprev1 -Oz -ffreestanding -fno-builtin -Werror -isystem "$base/include" -I "$target/src" -I "$target/hal" -I fw/build/smk37-elite/gen -I recovery/source/forward-tree/experiments/original_synth -I recovery/source/forward-tree/experiments/wobble -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_SYNTHONE_PATCH=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_AUTOPAN_PROBE=1 -DX0X_S1_AUTOPAN_PORT=1 -DX0X_SJ6_MONO32_PROBE=1 -DX0X_SJ6_NATIVE_RANDOM2=1 -DX0X_SJ6_NATIVE_POSITIVE_GATE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DX0X_S1_DELAY_TRACK_TEST=1 -DX0X_SJ6_FILTER_VEL_PROBE=1 -DX0X_SJ6_VELOCITY_PORT=1 -DX0X_SJ6_NATIVE_ZERO_GATE=$z -ferror-limit=0 -DX0X_ARP_HOOKS=1 -c "$target/src/app/engine.c" -o "recovery/source/engine-$z.o"
done
