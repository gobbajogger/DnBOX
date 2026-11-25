#!/bin/bash
set -euo pipefail
W=$(realpath "$(dirname "$0")")
R=$(realpath "$W/../../..")
TC="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
G=$(dirname "$(find "$R/fw/firmware/src" -name crc32_nibble.h | head -1)")
mkdir -p "$W/out"
exec > >(tee "$W/out/receipt.log") 2>&1
for o in s z; do
  for f in journal baseline; do
    "$TC/bin/cc" -O"$o" -mcpu=r3 -mfprev1 -ffp-contract=off -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -Wall -Werror -idirafter "$TC/include" -I "$G" -c "$W/$f.c" -o "$W/out/$f-$o.o"
  done
  for mode in crc selector full; do
    case $mode in
      crc) input=baseline; args=(-e vmj_fixture_crc);;
      selector) input=journal; args=(-e vmj_select);;
      full) input=journal; args=(-e vmj_select -u vmj_encode);;
    esac
    e="$W/out/$mode-$o.elf"
    "$TC/bin/ld" --gc-sections --relax "${args[@]}" -T "$W/codec.ld" -Map "$W/out/$mode-$o.map" "$W/out/$input-$o.o" -o "$e"
    echo "$mode -O$o"
    size -A "$e"
    sha256sum "$e"
    "$TC/bin/objdump" -d "$e" > "$W/out/$mode-$o.dis"
  done
done
