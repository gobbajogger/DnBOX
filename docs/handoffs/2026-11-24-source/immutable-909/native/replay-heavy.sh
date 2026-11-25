#!/bin/bash
set -euo pipefail
root=$1; w=$2; d=$root/full-gate-inputs/d9-native-reloc-wheel
h=$root/perf-final-proof/init-fdata-id3-heavy
b=$root/perf-final-proof/controls-init-only/source-wheel-native-scale
export PYTHONDONTWRITEBYTECODE=1
setdir="$w/src/labout"; (cd "$setdir";sha256sum {elite,pro}/never-install.elf >elf.sha256)
for m in elite pro;do
  wanted=$(awk '/^guarded / {print $NF}' "$d/$m.link.receipt")
  sha256sum "$setdir/$m/never-install.elf" | grep -q "$wanted"
done
e=$(mktemp -d /tmp/d9-heavy-wheel-seed.XXXXXX)
trap 'rm -rf -- "$e"' EXIT
ln -s "$e" "$w/seeds"
ln -s "$setdir" "$w/run-fdata"
python "$d/seed-non-user.py" "$root" "$setdir" "$e" >"$w/results/seed.log" 2>&1
# Authenticated production storage.c seeds exactly this public family tag;
# no real device ownership state. Adoption later demands exact native Sound.
printf 'SMK37DATAX0X\000\000\000\000' >"$w/native-owner-tag.bin"
cp -a "$b/build/gen" "$w/src/build/"
bash "$h/build-project.sh" "$root" "$w/src" > "$w/results/project.log" 2>&1 || { tail -20 "$w/results/project.log";exit 1; }
mv "$w/src/project.bin" "$w/project.bin"
python "$h/run-synthetic.py" "$root" "$w" "$w/results" s1-sj6-max909 elite:0 pro:0
# Same frozen workload, ordinary stock context; no stress attribution from
# this optional reference, since group and seeded project differ.
python "$h/run-synthetic.py" "$root" "$w" "$w/results" stock elite:0 pro:0
if [[ ${D9_RUN_909_PAIR:-0} == 1 ]];then
 mkdir -p "$w/paired-off" "$w/paired-on"
 cp "$w/results/seed.log" "$w/paired-off/seed.log"
 cp "$w/results/seed.log" "$w/paired-on/seed.log"
 cp "$h/run-synthetic.py" "$w/paired.py"
 patch --fuzz=0 "$w/paired.py" < "$d/paired-909-off.patch"
 for state in off on;do
   case $state in off)v=1;;on)v=0;;esac
   D9_DIAG_OFF="$v" python "$w/paired.py" "$root" "$w" "$w/paired-$state" s1-sj6-max909 pro:0
 done
 python - "$w/paired-on/s1-sj6-max909-pro-0.json" "$w/paired-off/s1-sj6-max909-pro-0.json" <<'PY'
import json,sys
a,b=[json.load(open(f)) for f in sys.argv[1:]]
for key in ('elf_sha256','scene_sha256','end_sound_sha256'): assert a[key]==b[key],key
assert [a['only_909_part_off'],b['only_909_part_off']]==[0,1]
assert a['initial_flash_sha256']==a['final_flash_sha256'] and b['initial_flash_sha256']==b['final_flash_sha256']
print("PASS matched executable, frozen scene, final Sound and enabled/disabled 909 state")
PY
fi
