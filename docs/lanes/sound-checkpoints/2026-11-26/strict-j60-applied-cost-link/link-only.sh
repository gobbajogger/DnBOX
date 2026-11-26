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
patch --fuzz=0 -s -p1 <"$W/warm-j60-applied-busy-top/proposal.patch"
patch --fuzz=0 -s -p1 <"$D/proposal.patch"
cd "$B"
sed -i '/b.CFLAGS.extend(Path(os.environ\[.DNBOX_CONTROL_FLAGS.\]).read_text().split())/a b.CFLAGS.extend(["-DDNBOX_S1_WET_LEASE_LAB=1","-DDNBOX_S1_WET_LEASE_USE=0","-DDNBX_EXPERIMENT_J60_POST=1", "-DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=1"])\nb.CFLAGS.extend(["-DDNBOX_LAB_MISSED_BUSY=1"])\nif os.environ.get("BUSY_WITNESS")=="1": b.CFLAGS.extend(["-DDNBOX_LAB_MISSED_COST=1"])' link.py
grep -n -E 'DNBOX_S1_WET_LEASE_LAB|BUSY_WITNESS' link.py
export W TMPDIR="$B" S1_PH_INLINE=0
for M in elite pro; do
 E="$W/warm-j60-applied-busy-top/link-receipts/$M-on.never-install.elf"
 for V in 0 1;do
  export BUSY_WITNESS="$V"
  L="$D/link-receipts/$M-$V.log"
  if ! bash cold-build.sh 2 "$M" >"$L" 2>&1; then tail -15 "$L"; exit 1; fi
  grep "^guarded $M" "$L"
  F="$B/tree/lab-cold-0-2/$M/never-install.elf"
  if [[ "$V" == 0 ]]; then
    cmp "$E" "$F" || { echo "OFF does not reproduce exact $M reference; stopping before ON" >&2; exit 2; }
  else
    cp "$F" "$D/link-receipts/$M-on.never-install.elf"
    python "$D/link-verify.py" "$E" "$F" "$M" >"$D/link-receipts/$M-on.json"
    cat "$D/link-receipts/$M-on.json"
  fi
 done
done
