#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd)
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp
T=$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
O=$(mktemp -d "$D/sj6-ota-history/preflight.XXXXXX"); trap 'rm -rf "$O"' EXIT; export TMPDIR=$O
test "$(sha256sum "$W/dnbx_core.c" | cut -d' ' -f1)" = 70d0aa0ea1cf2192e50cd51b45dffe01533cde368c0a1778959d39312b0ce759
test "$(sha256sum "$W/fastmath.h" | cut -d' ' -f1)" = c82737d5302d77b974d5071c9bc58915aca723a6e6f5f5185cbe43bad962189d
sed -n '/^static inline float sj6_ota_stage(/,/^}$/p' "$W/dnbx_core.c" | sed 's/static inline float /float /' >"$O/stage.inc"
printf '#include "fastmath.h"\n#include "stage.inc"\n' >"$O/a.c"
printf 'extern float fm_tanhf(float);\n#define sj6_ota_stage fallback_shared\n#include "stage.inc"\n' >"$O/b.c"
printf '#define fm_tanhf fm_tanhf_body\n#include "fastmath.h"\n#undef fm_tanhf\n__attribute__((noinline)) float fm_tanhf(float x){return fm_tanhf_body(x);}\n' >"$O/c.c"
for file in a b c; do
 "$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W" -c "$O/$file.c" -o "$O/$file.o"
 echo "$file"; "$T/bin/nm" -S --size-sort "$O/$file.o"
 cc -Os -ffp-contract=off -I"$W" -c "$O/$file.c" -o "$O/$file.host.o"
done
cc -Os -ffp-contract=off "$(dirname "$0")/verify.c" "$O/"*.host.o -o "$O/bench"
"$O/bench"
