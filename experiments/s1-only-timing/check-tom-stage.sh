#!/bin/sh
# Independent host proof against an equal tree with the original 909 source.
# Pass: <candidate firmware tree with Elite generated headers> <baseline drum909.c>
set -eu
tree=$1
base=$2
fixture_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT HUP INT TERM
cd "$tree"
python3 - "$scratch" "$base" <<'PY'
import pathlib, re, sys
p = pathlib.Path(sys.argv[1])
s = pathlib.Path(sys.argv[2]).read_text()
s = re.sub(r'^#define X0X_(DRUM|IMA)_TABLES_DEFINE.*\n', '', s, flags=re.M)
s = s.replace('"../x0x_cold.h"', '"x0x_cold.h"')
h = pathlib.Path('firmware/src/dsp/drum909.h').read_text()
names = sorted(set(re.findall(r'\b(drum909_\w+)\s*\(', h)))
(p/'rename.h').write_text(''.join(f'#define {n} ref_{n}\n' for n in names) +
                         '#define x0x_dist_names ref_x0x_dist_names\n')
(p/'ref.c').write_text(s)
PY
for opt in 0 2; do
 F="-O$opt -std=c99 -DX0X_HOST -ffp-contract=off -fno-fast-math -fno-builtin -Wall -Wextra -Werror -Ifirmware/src -Ifirmware/src/dsp -Ibuild/smk37-elite/gen"
 "${CC:-cc}" $F -include "$scratch/rename.h" -c "$scratch/ref.c" -o "$scratch/ref.o"
 "${CC:-cc}" $F -c firmware/src/dsp/drum909.c -o "$scratch/new.o"
 "${CC:-cc}" $F tests/host/drum909_emit_equivalence.c firmware/src/dsp/x0x_param.c "$scratch/ref.o" "$scratch/new.o" -lm -o "$scratch/equiv"
 "$scratch/equiv"
done
if [ "${BENCH_TOMS:-0}" = 1 ]; then
 "${CC:-cc}" $F "$fixture_dir/time-tom-stage.c" firmware/src/dsp/x0x_param.c \
  "$scratch/ref.o" "$scratch/new.o" -lm -o "$scratch/time"
 "$scratch/time"
fi
