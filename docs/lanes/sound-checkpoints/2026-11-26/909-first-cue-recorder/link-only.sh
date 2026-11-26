#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); W=$(dirname "$D"); J="$W/sj6-j60-right-native"
P=/tmp/phaser-warm-own.Y0XNbQ
B=$(mktemp -d "$D/build-only.XXXX"); trap 'rm -rf "$B"' EXIT
mkdir -p "$D/link-receipts" "$B/tree/experiments"
cd "$P/tree"
sha256sum lab-cold-0-2/{elite,pro}/never-install.elf
cp "$P/"{cold-build.sh,link.py} "$B/"
for p in firmware tools;do cp -as "$P/tree/$p" "$B/tree/";done
for p in smk37-elite smk37;do mkdir -p "$B/tree/build/$p"; ln -s "$P/tree/build/$p/gen" "$B/tree/build/$p/gen"; done
export PYTHONDONTWRITEBYTECODE=1
for f in $(awk '/^--- a\//{sub(/^a\//,"",$2);print $2}' "$J/"{wet-lease,table-share,j60-post}.patch "$D/proposal.patch" | sort -u); do
 dst="$B/tree/$f"
 test -L "$dst"
 cp --dereference "$dst" "$dst.edit"
 mv -Tf "$dst.edit" "$dst"
done
for p in assets reference ble;do ln -s "$P/tree/$p" "$B/tree/$p";done
cp -as "$P/tree/experiments/"{original_synth,wobble} "$B/tree/experiments/"
ln -s "$P/tree/experiments/sound-hardware-v1" "$B/tree/experiments/sound-hardware-v1"
cd "$B/tree"
for p in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 <"$J/$p.patch";done
python "$J/apply.py" firmware/src/dsp/juno6_j60_native.inc "$W/sj6-j60-read/right-stretch-host.patch"
patch --fuzz=0 -s -p1 <"$D/proposal.patch"
cd "$B"
sed -i '/b.CFLAGS.extend(Path(os.environ\[.DNBOX_CONTROL_FLAGS.\]).read_text().split())/a b.CFLAGS.extend(["-DDNBOX_S1_WET_LEASE_LAB=1","-DDNBOX_S1_WET_LEASE_USE=0","-DDNBX_EXPERIMENT_J60_POST=1", "-DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=1","-DDNBOX_LAB_FIRST_CUE=1"])' link.py
grep -n -E 'DNBOX_S1_WET_LEASE_LAB' link.py
export W TMPDIR="$B" S1_PH_INLINE=0
for M in elite pro;do
 L="$D/link-receipts/$M-on.log"
 if ! bash cold-build.sh 2 "$M" >"$L" 2>&1; then tail -18 "$L"; exit 1; fi
 grep "^guarded $M" "$L"
 F="$B/tree/lab-cold-0-2/$M/never-install.elf"
 cp "$F" "$D/link-receipts/$M-on.never-install.elf"
 # Do not retain any disposable outputs other than quarantined linked pin.
 rm -rf "$B/tree/lab-cold-0-2/$M"
 # Rebuildable private build .o only; never touch gen symlink or immutable donor.
 find "$B/tree/build" -type f -name '*.o' -delete
done
