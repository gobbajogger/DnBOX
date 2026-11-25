# Opt-in read-only typed controls carrier on exact 893

Apply native-trial-carrier.patch at the verified 893 RANDOM2/24-page
combined source. PREIMAGE/POSTIMAGE pin the six changed source files and
new header. Scope is native compile admission only, not new storage
format, final firmware, a deploy knob, or validated real hardware use.
The guarded host and older ordinary branches retain their parsers. No
source patch to AutoPan DSP or its floating-point cadence is included.
The typed tuple uses the same acknowledged FNV64 shed-of-BT-bit binding
and existing limits, lookup count and fallback, not an authentication
scheme. No row or object size changes.

Required explicit native flags, on matched dual-Init Synth One/SuperJ6
build flags:
    -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1
    -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1
Keep X0X_HOST **unset**. No stock env/default creates or activates typed
data. Existing trial-compatible S1 A4, Transpose, PB, MOD and reverb size
use already-built receiver routes. AutoPan is optional via
-DX0X_S1_AUTOPAN_PROBE=1, **only** with the
separate actual target AutoPan implementation; this patch does not
remove the AutoPan receiver hard error. The incompatible host-only
J6 Delay/Bend/RANDOM2/zero gate/filter-velocity/Classic and S1 clock/
crush/feedback profile flags are fail-closed. Unsupported nonzero
sidecar fields and nonzero SJ6 Filter Velocity lock bytes are refused,
not silently played. Native admission without the AutoPan flag
also refuses nonzero AutoPan bytes.

The trial disables all project/independent BT persistence and schedules
no project autosaves, even for ordinary input. It does not make the
entire device generally read-only; no physical device tests are allowed.
A fully coherent, explicitly admitted typed project is editable and
plays in RAM. A recovered older coherent tuple is playable but not
editable or saveable. Unsupported or torn-without-recovery remains
refused, rather than adopting, converting or writing defaults. Parser
validation and existing import/hold functions are retained.

For the safe emulator: initialize with a host-generated synthetic
project from an isolated store, or use the existing explicit validated
ED_PROJECT/portable import (project_file_validate then ui_project_end(1)
on a successfully acquired stage). No boot promotion. The supplied
emulator-readonly-autopan.x0xp is 32-byte X0XP header + project_t bytes,
both owners S1 Init with amount 32/48 and original sync-active accepted
target frequencies. The companion virtual-store .bin has 6 records:
u32 little-endian length then payload, in SET, SOUND, PAT0, PAT1,
SONG, MOTION order. The .bin is **not** a physical firmware flash image.
Use only on disposable emulator store. SYNTHETIC.sha256 pins these
fully synthetic generated inputs; neither contains presets/user data.

Evidence: same complete unchanged 893 mixed host fixture, independent
writer-to-trial real portable/flash admission (both owner SMK detents
and actual playback), old-reader refusal with no writes, true A/B
fallback, definitive no-prior/same-version torn refusal, actual gates
against manual/autosave/device BT writes. Sample amount edit is verified
from binding→SMK→owner receiver, not from label. GCC no-X0X_HOST syntax
tests for changed app boundaries and exact preprocessed stock project C
comparison do not substitute for JieLi linker, XIP/POOL, native target
timing or boot/audio qualification. No native slot/device/releases run
by this lane. The separate native JieLi smoke is root-owned.

Run `bash replay.sh` (or supply exact source-mod-wheels snapshot path).
Scratch uses a private workspace subdirectory, not global /tmp for trees.
