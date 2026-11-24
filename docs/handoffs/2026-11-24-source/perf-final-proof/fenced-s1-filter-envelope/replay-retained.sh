#!/usr/bin/env bash
set -euo pipefail
packet=$(realpath "$(dirname "$0")")
[[ $# == 2 ]] || { echo 'Usage: replay-retained.sh MAIN_PRIVATE_SCRATCH AUTHENTICATED_S1_PACK' >&2; exit 2; }
w=$(realpath "$1")
export S1_EDITOR_TEST_PACK=$(realpath "$2")
printf '%s  %s\n' "$(cat "$packet/S1-PACK.sha256")" "$S1_EDITOR_TEST_PACK" | sha256sum -c --status
cp "$packet"/ui_synthone_retained_{decay,fm}_test.c "$packet/ui_s1_filter_env_authored_test.c" "$packet/ui_synthone_legato_test.c" "$w/join/tests/host/"
sed '/^int main(void)$/,$d' "$packet/ui_synthone_filter_env_route_test.c" > "$w/join/tests/host/s1-filter-env-authored-helpers.h"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
profile() {
 (cd "$w/join"; SMC_UI_OUT="$w/$1" SMC_UI_TEST_FILE="tests/host/$2.c" sh tests/host/sj6-smc-ui.sh) >"$w/$1.log" 2>&1
 grep -E '^(PASS|owner|part)' "$w/$1.log"
}
profile authored ui_s1_filter_env_authored_test
profile retained-decay ui_synthone_retained_decay_test
profile retained-fm ui_synthone_retained_fm_test
(
  cd "$w/join"
  p=firmware/src/dsp/s1c/s1c.c
  cp "$p" "$w/dsp-legato-preimage"
  trap 'mv -f -- "$w/dsp-legato-preimage" "$p"' EXIT
  python3 - "$packet/instrument-host-legato.py" <<'PY'
# Primary harness already owns the engine.c peek; apply only temporary DSP test instrumentation.
import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
exec('from pathlib import Path\n'+s[s.index("p = Path('firmware/src/dsp/"):])
PY
  X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_COMBINED_LEVEL=1" profile retained-legato ui_synthone_legato_test
)
