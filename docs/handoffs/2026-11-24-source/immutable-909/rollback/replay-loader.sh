#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
root=$(realpath "$1")
t=$(mktemp -d "${TMPDIR:-/tmp}/dn-relocation.XXXXXX")
trap 'rm -rf -- "$t"' EXIT HUP INT TERM
python3 - "$root/firmware/loader/ldr_core.c" "$t/selected_loader.inc" <<'PY'
from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
out=[]
for name in ('ldr_put','ldr_region'):
 m=re.search(r'^static int '+name+r'\(',s,re.M)
 assert m, name
 start=s.index('{',m.start())
 n=1;end=start+1
 while n:
  n+=(s[end]=='{')-(s[end]=='}')
  end+=1
 out.append(s[m.start():end])
Path(sys.argv[2]).write_text('\n'.join(out)+'\n')
PY
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -I"$t" "$p/loader_pipe.c" -o "$t/check"
"$t/check"
