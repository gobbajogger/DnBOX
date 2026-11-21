#!/bin/sh
# Run from a DnBOX tree containing the original Miami stereo test.
# Uses the GNU linker's --wrap on Linux; never edits firmware sources.
set -eu
root=$(pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cat > "$work/mono.c" <<'C'
#include "dsp/original_miami_backend.h"
#include <string.h>
int __real_original_miami_backend_render_stereo(original_miami_backend *,
    original_miami_backend_workspace *, float *, float *, uint32_t);
int __wrap_original_miami_backend_render_stereo(original_miami_backend *part,
    original_miami_backend_workspace *workspace, float *left, float *right, uint32_t frames)
{
    int ok = __real_original_miami_backend_render_stereo(part, workspace, left, right, frames);
    if (ok) memcpy(right, left, frames * sizeof *left);
    return ok;
}
C
cat > "$work/cc" <<'SH'
#!/bin/sh
set -eu
exec ${MIAMI_BASE_CC:-cc} "$@" "$MIAMI_MONO_SOURCE" -Wl,--wrap=original_miami_backend_render_stereo
SH
chmod +x "$work/cc"
export MIAMI_MONO_SOURCE="$work/mono.c" MIAMI_BASE_CC="${CC:-cc}"
rc=0
CC="$work/cc" X0X_TEST_JOBS=1 sh tests/run_tests.sh miami-stereo > "$work/dispatcher.log" 2>&1 || rc=$?
if [ "$rc" -ne 1 ] || ! grep -F 'Stereo failed to increase side/mid RMS' "$root/build/host/miami-stereo.log" >/dev/null; then
    cat "$work/dispatcher.log"
    exit 1
fi
cat "$root/build/host/miami-stereo.log"
