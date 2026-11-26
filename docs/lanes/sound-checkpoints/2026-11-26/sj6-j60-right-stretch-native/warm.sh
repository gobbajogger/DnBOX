#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); W=$(dirname "$D")
P=${SJ6_STRICT_WARM_SOURCE:-/tmp/phaser-warm-own.Y0XNbQ}; B=$(mktemp -d "$D/warm.XXXX")
trap 'rm -rf "$B"' EXIT
cd "$P/tree"; sha256sum lab-cold-0-2/{elite,pro}/never-install.elf
mkdir "$B/tree"; cp "$P/"{cold-build.sh,link.py} "$B/"
for p in firmware tools build;do cp -a --reflink=auto "$P/tree/$p" "$B/tree/";done
for p in assets reference ble;do ln -s "$P/tree/$p" "$B/tree/$p";done
mkdir "$B/tree/experiments"
cp -a "$P/tree/experiments/"{original_synth,wobble} "$B/tree/experiments/"
for p in sound-hardware-v1;do ln -s "$P/tree/experiments/$p" "$B/tree/experiments/$p";done
cd "$B/tree"
for patch in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 <"$D/$patch.patch";done
python3 "$D/apply.py" firmware/src/dsp/juno6_j60_native.inc "$W/sj6-j60-read/right-stretch-host.patch"
cd "$B"
sha256sum cold-build.sh link.py "$D/"{wet-lease,table-share,j60-post}.patch "$W/all-controls-remote-join/compile-flags.txt"
sed -i '/b.CFLAGS.extend(Path(os.environ\[.DNBOX_CONTROL_FLAGS.\]).read_text().split())/a b.CFLAGS.extend(["-DDNBOX_S1_WET_LEASE_LAB=1","-DDNBOX_S1_WET_LEASE_USE=0","-DDNBX_EXPERIMENT_J60_POST=1", "-DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY="+os.environ["STAGE"]])' link.py
grep -n 'DNBOX_S1_WET_LEASE_LAB' link.py
export W TMPDIR="$B" S1_PH_INLINE=0
for M in elite pro;do
 for V in 0 1;do
  STAGE="$V" bash cold-build.sh 2 "$M"
  O="$D/out/warm-$M/$V";mkdir -p "$O"
  cp "tree/lab-cold-0-2/$M/"{never-install.elf,lab.map,text.bin} "$O/"
 done
done
