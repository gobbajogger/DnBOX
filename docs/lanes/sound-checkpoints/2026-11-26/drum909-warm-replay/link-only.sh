#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); W=$(dirname "$D"); J="$W/sj6-j60-right-native"
P=/tmp/phaser-warm-own.Y0XNbQ
B=$(mktemp -d "$D/build-only.XXXX"); trap 'rm -rf "$B"' EXIT
mkdir -p "$D/link-receipts" "$B/tree/experiments"
cd "$P/tree"
sha256sum lab-cold-0-2/{elite,pro}/never-install.elf
cp "$P/"{cold-build.sh,link.py} "$B/"
for p in firmware tools build;do cp -a --reflink=auto "$P/tree/$p" "$B/tree/";done
for p in assets reference ble;do ln -s "$P/tree/$p" "$B/tree/$p";done
cp -a "$P/tree/experiments/"{original_synth,wobble} "$B/tree/experiments/"
ln -s "$P/tree/experiments/sound-hardware-v1" "$B/tree/experiments/sound-hardware-v1"
cd "$B/tree"
for p in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 <"$J/$p.patch";done
python "$J/apply.py" firmware/src/dsp/juno6_j60_native.inc "$W/sj6-j60-read/right-stretch-host.patch"
cd "$B"
sed -i '/b.CFLAGS.extend(Path(os.environ\[.DNBOX_CONTROL_FLAGS.\]).read_text().split())/a b.CFLAGS.extend(["-DDNBOX_S1_WET_LEASE_LAB=1","-DDNBOX_S1_WET_LEASE_USE=0","-DDNBX_EXPERIMENT_J60_POST=1", "-DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=1"])' link.py
grep -n -E 'DNBOX_S1_WET_LEASE_LAB' link.py
export W TMPDIR="$B" S1_PH_INLINE=0
o="$B/tree/firmware/src/dsp/drum909.c"
echo "69e6f18b56ef6c710ed018dd352d5c79eac5faabc1fe653b40606287c8a6c5d7  $o" | sha256sum -c -
for V in 0 1; do
 if [[ "$V" == 1 ]]; then
  (cd "$B/tree" && patch --fuzz=0 -s -p1 <"$D/proposal.patch")
  echo "4c6ce4a0a708e67600038aedd4508b0178bf05492bf19ac94d3fa382b936e01d  $o" | sha256sum -c -
 fi
 for M in elite pro; do
  E="$J/out/warm-$M/1/never-install.elf"
  L="$D/link-receipts/$M-$V.log"
  if ! bash cold-build.sh 2 "$M" >"$L" 2>&1; then tail -15 "$L"; exit 1; fi
  grep "^guarded $M" "$L"
  F="$B/tree/lab-cold-0-2/$M/never-install.elf"
  if [[ "$V" == 0 ]]; then cmp "$E" "$F"; fi
  sha256sum "$F"
  if [[ "$V" == 1 ]]; then cp "$F" "$D/link-receipts/$M-on.never-install.elf"; fi
 done
done
