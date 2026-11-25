#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")");root=$(realpath "$1"); old=$(realpath "$2")
t=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-paired.XXXXXX")
trap 'rm -rf -- "$t"' EXIT HUP INT TERM
test "$(git hash-object "$root/tests/ldr_test.c")" = 9e4dec9c6f7b1238edcfaf2c7b2af968c7e8c502
test "$(git hash-object "$root/firmware/loader/ldr_core.c")" = d27ba30ce3062cf46d8cd30d462bc6b04d9f7eba
test "$(git hash-object "$root/firmware/src/ota.c")" = af42a32311ec8ea41167dd0b5abbc1fbeeeba506
mkdir "$t/tests"
ln -s "$root/firmware" "$t/firmware"
python3 - "$root/tests/ldr_test.c" "$t/tests/loader_harness.inc" <<'PY'
import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
a="static int ldr_prog(uint32_t off, const void *p, uint32_t n)"
s=s.replace(a,"static unsigned repeat_fd_fault;\n"+a)
a="    for (i = 0; i < n; i++) nor[off + i] &= s[i];"
s=s.replace(a,"    if(repeat_fd_fault && off == repeat_fd_fault) {\n        for(i=0;i<n/2;i++) nor[off+i]&=s[i];\n        repeat_fd_fault=0;return -1;\n    }\n"+a)
Path(sys.argv[2]).write_text(s)
PY
"${CC:-cc}" -std=c11 -O2 -DBOARD_SMK37=1 -fsanitize=undefined -fno-sanitize-recover=all -I"$t/tests" "$p/paired_loader.c" -o "$t/check"
"$t/check" "$old"
