#!/bin/bash
set -eu
H=$(cd "$(dirname "$0")";pwd)
B=$(cd "$1";pwd)
T=$B/tree
CFLAGS=(-DX0X_HOST=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_ARP_ENGINE=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DDNBOX_137_HOST_WITNESS=1 -DX0X_DB=1 -D_POSIX_C_SOURCE=200809L)
readarray -t defs < <(grep -oE -- '-D[^[:space:]]+' "$B/compose-flags.txt" |sort -u)
cd "$T"
readarray -t units < <(X0X_S1C=1 X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 PYTHONDONTWRITEBYTECODE=1 python - <<'PY'
from tools import build as b
print(*[p for p in b.ORIGINAL_SYNTH_UNITS+b.ORIGINAL_SYNTH_BACKENDS if not p.endswith(('s1pack.c','original_runtime.c'))],sep='\n')
PY
)
units+=(firmware/src/dsp/drum909.c "$H/original_runtime.c")
printf '%s\n' "${CFLAGS[@]}" "${defs[@]}" > "$H/host.flags"
sha256sum "${units[@]}" "$H/render.c" "$H/cue.h" > "$H/host-source.sha256"
cc -include math.h -std=gnu11 -O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections "${CFLAGS[@]}" "${defs[@]}" \
 -I"$H" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Iexperiments/original_synth -Iexperiments/wobble -Ibuild/smk37/gen -Ibuild/gen -DORIGINAL_USE_SOFT_FMA -DMIAMI_FX_NO_HOST_FENV -DX0X_FAST_FMA "$H/render.c" "${units[@]}" -Wl,--gc-sections -lm -o "$H/render"
sha256sum "$H/render"
