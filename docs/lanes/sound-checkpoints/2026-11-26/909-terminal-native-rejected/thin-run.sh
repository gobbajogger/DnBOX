#!/bin/bash
set -euo pipefail
L=$(cd "$(dirname "$0")"&&pwd); W=$(dirname "$L"); D="$W/sj6-j60-right-native"; M=${1:?elite_or_pro}; R=${2:-1}; [[ $R == 0 || $R == 1 ]]; export DNBX_TERMINAL909_SWITCH="$R"
P=${SJ6_STRICT_WARM_SOURCE:-/tmp/phaser-warm-own.Y0XNbQ}; B=$(mktemp -d "$L/native-$M.XXXX"); [[ "$M" == elite || "$M" == pro ]]
trap 'rm -rf "$B"' EXIT
export TMPDIR="$B"
cd "$P/tree"; sha256sum lab-cold-0-2/{elite,pro}/never-install.elf
mkdir "$B/tree"; cp "$P/"{cold-build.sh,link.py} "$B/"
python3 "$L/thin-link.py" "$P/tree" "$B"
mkdir "$B/tree/tools"
cp "$P/tree/tools/"{build.py,check_locked_settings.py} "$B/tree/tools/"
for dep in fm1pkg_make.py gen_cache.py lz4blk.py rom_guard.py check_original_inputs.py firmware_build_labels.py fm1_install.py locked_settings.json;do
  ln -s "$P/tree/tools/$dep" "$B/tree/tools/$dep"
done
export PYTHONDONTWRITEBYTECODE=1
G=smk37; [[ $M != elite ]] || G=smk37-elite
mkdir -p "$B/tree/build/$G"
ln -s "$P/tree/build/$G/gen" "$B/tree/build/$G/gen"
cp "$P/tree/build/$G/app.ld" "$B/tree/build/$G/app.ld"
for patch in wet-lease table-share j60-post;do
 while read -r file; do
   [[ "$file" == a/* ]] || continue
   file=${file#a/}; rm -- "$B/tree/$file"; cp "$P/tree/$file" "$B/tree/$file"
 done < <(awk '/^--- /{print $2}' "$D/$patch.patch")
done
if [[ $R == 1 ]]; then
 for f in firmware/src/{app/engine.c,dsp/drum909.c,dsp/drum909.h};do
  unlink "$B/tree/$f"
  cp "$P/tree/$f" "$B/tree/$f"
 done
 # original_runtime.h was already copied for the earlier warm wet-lease
 # patch. Apply our exact donor patch before any such overlapping header patch.
 (cd "$B/tree"; patch --fuzz=0 -p1 < "$L/diagnostic.patch")
fi
for p in assets reference ble;do ln -s "$P/tree/$p" "$B/tree/$p";done
mkdir "$B/tree/experiments"
cp -a "$P/tree/experiments/wobble" "$B/tree/experiments/"
mkdir "$B/tree/experiments/original_synth"
for src in "$P/tree/experiments/original_synth/"*;do
 if [[ "$src" == */motion_tables_private.h ]]; then
  ln -s "$src" "$B/tree/experiments/original_synth/"
 else
  cp -a "$src" "$B/tree/experiments/original_synth/"
 fi
done
for p in sound-hardware-v1;do ln -s "$P/tree/experiments/$p" "$B/tree/experiments/$p";done
cd "$B/tree"
for patch in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 <"$D/$patch.patch";done
python3 "$D/apply.py" firmware/src/dsp/juno6_j60_native.inc "$W/sj6-j60-read/right-stretch-host.patch"
cd "$B"
# build.py's tool wrapper resolves Path symlinks before relativizing. Keep
# arguments lexical within this isolated source snapshot, so outside
# donors are only read through the authorized links, never compiled in place.
python3 - <<'LEX'
from pathlib import Path
p=Path("link.py");s=p.read_text()
a="real=b.tc"
b="""toolchain_call=b.tc
def real(tool,*args):
    p=Path.cwd()
    a=[os.path.relpath(os.path.abspath(x),p) if isinstance(x,Path) else x for x in args]
    return toolchain_call(tool,*a)
"""
assert s.count(a)==1
p.write_text(s.replace(a,b))
LEX
sha256sum cold-build.sh link.py "$D/"{wet-lease,table-share,j60-post}.patch "$W/all-controls-remote-join/compile-flags.txt"
sed -i '/b.CFLAGS.extend(Path(os.environ\[.DNBOX_CONTROL_FLAGS.\]).read_text().split())/a b.CFLAGS.extend(["-DDNBOX_S1_WET_LEASE_LAB=1","-DDNBOX_S1_WET_LEASE_USE=0","-DDNBX_EXPERIMENT_J60_POST=1", "-DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=1","-DX0X_DIAG_909_TERMINAL_GROUP="+os.environ["DNBX_TERMINAL909_SWITCH"]])' link.py
grep -n 'DNBOX_S1_WET_LEASE_LAB' link.py
export W TMPDIR="$B" S1_PH_INLINE=0

bash cold-build.sh 2 "$M"
ELF="tree/lab-cold-0-2/$M/never-install.elf"
O="$L/thin-receipts"; mkdir -p "$O"
expected=aa2cacb33abf17bf35b287db214f82185f0bbc8a226f2daad63fb814ce66ac69
if [[ $M == pro ]]; then expected=34537d270385218c6eddc08ccf1d752a558a69c39385db7746c3867c594beb86; fi
if [[ $R == 0 ]]; then
 printf '%s  %s\n' "$expected" "$ELF" | sha256sum -c -
 echo "$M exact thinner OFF reproduced $expected"
 touch "$O/$M.confirmed-off"
else
 [[ -f "$O/$M.confirmed-off" ]]
 # The link completed. Delete only this arm's temporary .o outputs once
 # they are no longer needed, to fit baseline decompression/guard reads.
 find "$B/tree/build/$G" -type f -name '*.o' -delete
 OFF="$B/off.elf"
 gzip -dc "$W/owner909-early-chicago/thin-receipts/$M-off.never-install.elf.gz" >"$OFF"
 printf '%s  %s\n' "$expected" "$OFF" | sha256sum -c -
 python3 "$L/thin-guard.py" "$OFF" "$ELF" "$M" candidate
 gzip -n -c "$ELF" >"$O/$M-on.never-install.elf.gz"
fi
sha256sum "$ELF" "tree/lab-cold-0-2/$M/lab.map"
gzip -n -c "tree/lab-cold-0-2/$M/lab.map" >"$O/$M-$R.map.gz"
