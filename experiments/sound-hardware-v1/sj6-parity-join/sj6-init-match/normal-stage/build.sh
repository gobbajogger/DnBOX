#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd)
B=$(cd "$1";pwd)
T=$(realpath "$2")
python "$D/stage_host.py" "$T"
base="$B/tree/firmware/src/dsp/dnbx_core.c"
python "$D/wire.py" "$base" "$T/dnbx_core.c"
diff -u --label a/firmware/src/dsp/dnbx_core.c --label b/firmware/src/dsp/dnbx_core.c "$base" "$T/dnbx_core.c" >"$D/normal-init.patch" || test "$?" = 1
CFLAGS=(-DX0X_HOST=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_ARP_ENGINE=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DDNBOX_137_HOST_WITNESS=1 -DX0X_DB=1 -D_POSIX_C_SOURCE=200809L)
readarray -t defs < <(grep -oE -- '-D[^[:space:]]+' "$B/compose-flags.txt" |sort -u)
cd "$B/tree"
readarray -t units < <(X0X_S1C=1 X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 PYTHONDONTWRITEBYTECODE=1 python - <<'PY'
from tools import build as b
print(*[p for p in b.ORIGINAL_SYNTH_UNITS+b.ORIGINAL_SYNTH_BACKENDS if not p.endswith(('s1pack.c','dnbx_core.c'))],sep='\n')
PY
)
units+=(firmware/src/dsp/drum909.c "$T/dnbx_core.c")
inc=(-I"$D/../../sj6-parity-suite/private" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Iexperiments/original_synth -Iexperiments/wobble -Ibuild/smk37/gen -Ibuild/gen)
cc -E -P -include math.h "${CFLAGS[@]}" "${defs[@]}" "${inc[@]}" "$base" >"$T/original.i"
cc -E -P -include math.h "${CFLAGS[@]}" "${defs[@]}" "${inc[@]}" "$T/dnbx_core.c" >"$T/default.i"
cmp "$T/original.i" "$T/default.i"
cc -include math.h -std=gnu11 -O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections "${CFLAGS[@]}" "${defs[@]}" "${inc[@]}" -DDNBOX_INIT_NORMAL_LAB=1 \
 -DORIGINAL_USE_SOFT_FMA -DMIAMI_FX_NO_HOST_FENV -DX0X_FAST_FMA "$T/render.c" "${units[@]}" -Wl,--gc-sections -lm -o "$T/render"
"$T/render" 0 /dev/null >"$T/stock.f32" 2>"$T/stock.log"
"$T/render" 1 "$T/tap.f32" >"$T/corrected.f32" 2>"$T/corrected.log"
cmp "$T/stock.log" "$T/corrected.log"
cmp -s "$T/stock.f32" "$T/corrected.f32" && { echo "expected a change in the instrument"; exit 1; }
"$T/render" 1 "$T/repeat-tap.f32" | cmp - "$T/corrected.f32"
cmp "$T/tap.f32" "$T/repeat-tap.f32"
