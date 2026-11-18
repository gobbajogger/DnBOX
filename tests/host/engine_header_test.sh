#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
# Deliberately no build/gen or generated tables; no ambient include-path escape.
unset CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsyntax-only tests/host/engine_header_test.c
# Dependency inspection also detects a regression if another runner generated
# tables earlier; public declarations must not inherit any private DSP owner.
deps=$("${CC:-cc}" -MM tests/host/engine_header_test.c)
if printf '%s\n' "$deps" | grep -E '(^|[/ ])(drum909(_dsp)?|juno6|x0x_drum_tables)\.h([ \\]|$)'; then
 echo 'FAIL: public engine header pulled private DSP/generated headers'; exit 1
fi
echo 'Public engine header: standalone value ABI and no private DSP dependency PASS'
