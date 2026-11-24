set -euo pipefail
root=/workspace/scratch/3a7051e761a0
proof="$(cd "$(dirname "$0")" && pwd)"
work="${2:-$(cat "$proof/scratch.txt")}"
base=${1:-/tmp/dn-fit-hz.e2mFXG}
pack="${3:-$root/s1-audit-data/s1.pack}"
test "$(sha256sum "$pack" | cut -d' ' -f1)" = c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44
mkdir -p "$work/factory"
for side in base candidate; do
  source="$base"
  if test "$side" = candidate; then source="$work"; fi
  (
    cd "$source"
    cc -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
      -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp \
      "$proof/factory-probe.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c \
      firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$work/factory/$side"
  )
  "$work/factory/$side" "$pack" "$work/factory/$side.pcm"
done
cmp "$work/factory/base.pcm" "$work/factory/candidate.pcm"
sha256sum "$work"/factory/*.pcm
printf 'PASS byte-exact factory comparison, %s bytes per image\n' "$(wc -c < "$work/factory/base.pcm")"
