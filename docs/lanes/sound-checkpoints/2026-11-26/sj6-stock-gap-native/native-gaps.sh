#!/bin/bash
set -euo pipefail
L=$(cd "$(dirname "$0")"&&pwd); W=$(dirname "$L"); D="$W/sj6-j60-right-native"; M=${1:?elite_or_pro}; R=${2:-1}; [[ $R == 0 || $R == 1 ]]; export DNBX_GAP_SWITCH="$R"
P=${SJ6_STRICT_WARM_SOURCE:-/tmp/phaser-warm-own.Y0XNbQ}; B=$(mktemp -d "$L/native-$M.XXXX"); [[ "$M" == elite || "$M" == pro ]]
trap 'rm -rf "$B"' EXIT
cd "$P/tree"; sha256sum lab-cold-0-2/{elite,pro}/never-install.elf
mkdir "$B/tree"; cp "$P/"{cold-build.sh,link.py} "$B/"
python3 "$L/thin-link.py" "$P/tree" "$B"
cp -a "$P/tree/tools" "$B/tree/tools"
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
for p in juno6.c juno6.h;do rm "$B/tree/firmware/src/dsp/$p"; cp "$P/tree/firmware/src/dsp/$p" "$B/tree/firmware/src/dsp/$p"; done
python3 "$L/wire-gaps.py" "$B/tree/firmware/src/dsp/juno6.h" "$B/tree/firmware/src/dsp/juno6.c" --retain-gaps
for p in assets reference ble;do ln -s "$P/tree/$p" "$B/tree/$p";done
mkdir "$B/tree/experiments"
cp -a "$P/tree/experiments/"{original_synth,wobble} "$B/tree/experiments/"
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
sed -i '/b.CFLAGS.extend(Path(os.environ\[.DNBOX_CONTROL_FLAGS.\]).read_text().split())/a b.CFLAGS.extend(["-DDNBOX_S1_WET_LEASE_LAB=1","-DDNBOX_S1_WET_LEASE_USE=0","-DDNBX_EXPERIMENT_J60_POST=1", "-DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=1","-DDNBX_SJ6_RECLAIM_STOCK_LAYOUT="+os.environ["DNBX_GAP_SWITCH"]])' link.py
grep -n 'DNBOX_S1_WET_LEASE_LAB' link.py
export W TMPDIR="$B" S1_PH_INLINE=0

bash cold-build.sh 2 "$M"
ELF="tree/lab-cold-0-2/$M/never-install.elf"
PYTHONPATH= python3 "$L/native-score.py" "$ELF" "$D/out/warm-$M/1/never-install.elf"
