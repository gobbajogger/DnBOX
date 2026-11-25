#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
u="$(dirname "$p")/s1-mod-smk-ui"
bend="$(dirname "$p")/s1-bend-typed"
id="$(dirname "$p")/id137-smk-filter"
rv="$(dirname "$p")/s1-init-reverb-lfo"
a=$(dirname "$p")
j="$a/sj6-id137-joined-carriage"
m="$a/s1-mod-rate-probe"
s="$a/s1-mod-sidecar"
g="$a/id137-generation-bind"
b=$(realpath "$1")
test "$(sha256sum "$b/../MOD-WHEELS-POST.sha256" | cut -d' ' -f1)" = b71f366f8da6ed85a64dbd3ba90b4c2784503511a01558c81ead2ee53e037e3d
(cd "$b"; sha256sum --quiet -c ../MOD-WHEELS-POST.sha256; sha256sum --quiet -c "$j/PREIMAGE.sha256")
t=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-s1-mod-sidecar.XXXXXX")
echo "$t" >"$p/isolated.txt"
cd "$t"
mkdir build out
for dir in firmware tests host; do cp -a --reflink=auto "$b/$dir" .; done
cp -a "$b/build/gen" build/
for dir in tools assets experiments ble; do ln -s "$b/$dir"; done
cp "$bend/s1_pb_mod_ui.c" "$j/sj6_137_carriage.c" "$rv/ui_s1_init_reverb_lfo_test.c" "$j/sj6_137_old.c" "$j/ui_vnext_old_usb_witness.inc" tests/host/
patch -s --fuzz=0 -p1 <"$j/combined-host.patch"
sha256sum --quiet -c "$j/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$m/cc1-rate-probe.patch"
patch -s --fuzz=0 -p1 <"$s/s1-mod-sidecar.patch"
patch -s --no-backup-if-mismatch --fuzz=0 -p1 <"$g/tuple-binding.patch"
patch -s --fuzz=0 -p1 <"$u/s1-mod-smk-ui.patch"
patch -s --fuzz=0 -p1 <"$bend/bend-ranges-typed.patch"
sha256sum --quiet -c "$bend/POSTIMAGE.sha256"

patch -s --fuzz=0 -p1 <"$g/torn-host-tests.patch"
patch -s --fuzz=0 -p1 <"$id/filter-mod-host.patch"
# Both add enum entries after PT_PITCH_MOD; keep PB before FILTER MOD.
sed '3,12d' "$id/filter-mod-ui.patch" | patch -s --fuzz=0 --no-backup-if-mismatch -p1
python3 - "$rv/s1-init-reverb-lfo.patch" >out/reverb-commuting.patch <<'FILTER'
from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
target=""
for block in re.split(r'(?=^--- |^@@ )',s,flags=re.M):
 if block.startswith('--- '):
  target=block.splitlines()[0]
 if (target.endswith('/s1_patch.h') and block.startswith('@@ -31,')) or (target.endswith('/s1c_part.c') and block.startswith(('@@ -231,','@@ -252,'))) or (target.endswith('/ui.c') and block.startswith('@@ -2423,')):
  continue
 sys.stdout.write(block)
FILTER
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <out/reverb-commuting.patch
python3 - <<'INNER'
from pathlib import Path
q=Path("firmware/src/app/ui.c")
s=q.read_text()
for old,new in (
 ('    PT_PITCH_MOD,','    PT_PITCH_MOD,\n#ifdef DNBOX_137_HOST_WITNESS\n    PT_FILTER_MOD,\n#endif'),
 ('{PT_FX_BYPASS, {S1_DELAY_ON, S1_REVERB_ON, ORIG_PAGE_NONE, ORIG_PAGE_NONE}},','{PT_FX_BYPASS, {S1_DELAY_ON, S1_REVERB_ON, S1_REVERB_MIX_LFO, ORIG_PAGE_NONE}},')):
 assert s.count(old)==1, old
 s=s.replace(old,new)
q.write_text(s)
q=Path("firmware/src/app/s1_patch.h");s=q.read_text()
# PB uses TREMOLO+1/+2; authored Reverb occupies the next synthetic role.
old="S1_TREMOLO_LFO,\n    S1_BASE_EDIT_FIRST"
assert s.count(old)==1
s=s.replace(old,"S1_TREMOLO_LFO, S1_REVERB_MIX_LFO = S1_TREMOLO_LFO+3,\n    S1_BASE_EDIT_FIRST")
q.write_text(s)
q=Path("firmware/src/app/s1c_part.c");s=q.read_text()
for old,new in (
 ("    if (slot == S1_CUTOFF_LFO) return &cutoff_lfo;", "    if (slot == S1_REVERB_MIX_LFO) return &reverb_lfo;\n    if (slot == S1_CUTOFF_LFO) return &cutoff_lfo;"),
 ("    if (id == P_CUTOFF_LFO) return S1_CUTOFF_LFO;", "    if (id == P_REVERB_MIX_LFO) return S1_REVERB_MIX_LFO;\n    if (id == P_CUTOFF_LFO) return S1_CUTOFF_LFO;")):
 assert s.count(old)==1
 s=s.replace(old,new)
q.write_text(s)
INNER
awk '/^static int smk_pitch_parameter\(/ {p=1} p {print} p && /^}/ {exit}' firmware/src/app/main_fm1.c >out/mod-wheels.inc
awk '/^static void smk_wheels\(void\)$/ {p=1} p {print} p && /^}/ {exit}' firmware/src/app/main_fm1.c >>out/mod-wheels.inc
