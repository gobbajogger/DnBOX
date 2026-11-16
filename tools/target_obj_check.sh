#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Compile one C file for the FM-1 (-mcpu=r3 -mfprev1 -O2) and report what the DSP
# rules forbid: calls into soft-double routines (__*df*), libm, malloc/free, and any
# undefined symbol outside an allow list. Also prints the instruction count of each
# function (a rough cost signal; tests/target_budget.py does the loop-weighted one).
#   tools/target_obj_check.sh firmware/src/dsp/bass303.c [extra cflags...]
# Needs JIELI_TOOLCHAIN (tools/get_toolchain.sh). Uses docker or podman when one
# is installed. Otherwise it runs the toolchain binaries on this machine.
set -e
cd "$(dirname "$0")/.."
SRCF="$1"; shift
: "${JIELI_TOOLCHAIN:?set JIELI_TOOLCHAIN}"
TC="$(cd "$JIELI_TOOLCHAIN" && pwd -P)"
DOCKER=""
if command -v docker >/dev/null 2>&1; then DOCKER=docker
elif command -v podman >/dev/null 2>&1; then DOCKER=podman
fi
OBJ="build/objcheck/$(basename "$SRCF" .c).o"
mkdir -p build/objcheck
if [ -n "$DOCKER" ]; then
    run() { "$DOCKER" run --rm --platform linux/amd64 -v "$PWD:/work" -v "$TC:/opt/jieli:ro" -w /work \
            debian:bookworm-slim "$@"; }
    CC=/opt/jieli/pi32v2/bin/clang
    NM=/opt/jieli/pi32v2/bin/nm
    OBJDUMP=/opt/jieli/common/bin/objdump
else
    echo "target_obj_check: no container; using $TC directly" >&2
    run() { "$@"; }
    CC="$TC/pi32v2/bin/clang"
    NM="$TC/pi32v2/bin/nm"
    OBJDUMP="$TC/common/bin/objdump"
    # The driver is common/bin/clang via a pi32v2 symlink, so it does not
    # search pi32v2/include on its own. Firmware sources include stdlib.h.
    set -- -idirafter "$TC/pi32v2/include" "$@"
fi
run "$CC" -target pi32v2 -mcpu=r3 -mfprev1 -O2 -fno-builtin -ffp-contract=off \
    -Wall -Wdouble-promotion -Ifirmware/src -Ibuild/gen "$@" -c "$SRCF" -o "$OBJ"
run sh -c "$NM -u $OBJ; echo ---; $OBJDUMP -d $OBJ" > "$OBJ.txt"
UNDEF="$(awk '/^---$/{exit} {print $NF}' "$OBJ.txt")"
BAD=""
for s in $UNDEF; do
    case "$s" in
        __*df*|__*sf2df*|__extendsfdf2|__truncdfsf2) BAD="$BAD $s(double)";;
        malloc|calloc|realloc|free) BAD="$BAD $s(alloc)";;
        expf|exp|tanhf|tanh|powf|pow|sinf|sin|cosf|cos|tanf|tan|logf|log|log2f|log10f|sqrtf|sqrt|floorf|floor|fabsf|fabs|fmodf|fmod|exp2f|roundf|lrintf|atanf|atan2f|ceilf)
            BAD="$BAD $s(libm)";;
        memset|memcpy|memcmp) ;;      # firmware/src/libc.c. Nothing else is linked.
        *) BAD="$BAD $s(undefined)";;
    esac
done
echo "functions (instructions):"
awk '/^[A-Za-z_][A-Za-z0-9_]*:$/{if(f)printf "  %-40s %d\n",f,n; f=substr($0,1,length($0)-1); n=0; next} /^ +[0-9a-f]+:/{n++} END{if(f)printf "  %-40s %d\n",f,n}' "$OBJ.txt" | sort -k2 -n -r | head -40
if [ -n "$BAD" ]; then echo "FORBIDDEN:$BAD"; exit 1; fi
echo "ok: no double, libm, alloc or unknown symbols in $SRCF"
