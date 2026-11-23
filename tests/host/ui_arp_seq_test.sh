#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# SJ6 / Synth One arp pages against the hooks fake, plus source mutants that must fail.
set -eu
cd "$(dirname "$0")/../.."
OUT=${1:-build/host/ui-arp-seq}
mkdir -p "$OUT"; M=$(cd "$OUT" && pwd)
export X0X_SYNTHONE_PATCH=1   # the shipping Elite/Pro Synth One pages
FAKE="-DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_ARP_FAKE_H=\"$PWD/tests/host/ui_arp_fake.h\""
run() { X0X_HOST_DEFS="$FAKE ${2:-}" SMC_UI_TEST_FILE=tests/host/ui_arp_seq_test.c SMC_UI_OUT="$OUT/$1" sh tests/host/smc_ui_test.sh; }
run base > "$OUT/base.log" 2>&1 || { cat "$OUT/base.log"; exit 1; }
tail -1 "$OUT/base.log"
# Hooks off (device default): no arp pages, the build still links.
X0X_HOST_DEFS="-DX0X_ARP_HOOKS=0" SMC_UI_TEST_FILE=tests/host/ui_list_feedback_test.c SMC_UI_OUT="$OUT/off" \
    sh tests/host/smc_ui_test.sh > "$OUT/off.log" 2>&1 || { cat "$OUT/off.log"; exit 1; }
mutant() {   # name, python replace (old, new) on ui.c
    name=$1; shift
    src="$M/mut-$name/ui.c"; mkdir -p "$OUT/mut-$name"
    python3 - "$src" "$@" <<'PY'
import sys
dst, old, new = sys.argv[1:4]
s = open('firmware/src/app/ui.c').read()
assert s.count(old) == 1, old
open(dst, 'w').write(s.replace(old, new))
PY
    if run "mut-$name" "-DX0X_HOST_UI_SOURCE=\"$src\"" > "$OUT/mut-$name.log" 2>&1; then
        echo "MUTANT SURVIVED $name"; exit 1
    fi
    grep -q "^FAIL" "$OUT/mut-$name.log" || { echo "mutant did not build/run: $name"; tail -5 "$OUT/mut-$name.log"; exit 1; }
    echo "mutant $name rejected"
}
mutant pads-to-pattern '        else if (w >= 0 && arp_seq_page())
            arp_step_key(w, down);' '        else if (0)
            arp_step_key(w, down);'
mutant effect-first '                if (kind == ORIGINAL_SOUND_SYNTHONE && row == count - 1 && arp_engine(b))' '                if (kind == ORIGINAL_SOUND_SYNTHONE && row == 0 && arp_engine(b))'
mutant fake-playhead '        ph = as.on && engine_arp_view((unsigned)b, &arp_ph, &age) && age < 250u ? (int)arp_ph : -1;' '        ph = engine_arp_view((unsigned)b, &arp_ph, &age) ? (int)arp_ph : -1;'
mutant led-playhead '                    if (live && ph == i)' '                    if (ph == i)'
mutant k8-edits '    }                                            /* K8: nothing on an arp step */' '    } else arp_step_edit((unsigned)arp_held, ARP_S_OCT, 1);'
echo "PASS ui-arp-seq (base + hooks-off + 5 mutants)"
