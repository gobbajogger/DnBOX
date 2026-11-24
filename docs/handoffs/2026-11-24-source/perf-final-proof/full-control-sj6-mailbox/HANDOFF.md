# SuperJ6 two-word knob mailbox

The source patch applies to the captured combined tree with A REF and the Synth One LFO join. The live combined tree matched the three source preimages at verification. The isolated checkout is under this packet. Apply the source and host-test patches to the same preimage. The apply receipt checks the files before and after both patches and verifies reverse application.

The mailbox uses two 32-bit words per part. Every queued slot selects its word and uses a bit index from 0 to 31. Foreground writes and reads retain the existing CQ lock. The audio boundary snapshots and clears both words, checks ownership, and applies the current low-word DSP and persistence groups in their existing order. The host upper-word probe applies in slot order after those groups. A REF stays in slot 31. The PWM BASE panel alias still follows the pending route and resolves to a low-word PWM slot.

Preset publication and cancellation retire the old part's queued values. Accepted factory replacement, transport note locks, and direct project adoption also retire them. A rejected factory row leaves the queue available, and a project save hold defers it. Each retirement test checks the opposite part separately. The baseline reproduced stale A REF after preset cancellation and after a note lock began and ended with edits held, for both part assignments. The same four reproductions passed after the change.

The host-only probe exercises slots 32 and 63 alongside A REF, oscillator, chorus, and PWM alias edits on sounding parts. It verifies pending readback, coalesced relative turns, commit order, slot bounds, save admission with only an upper-word edit, and persistence of the named controls. Shift sanitization stayed clean across ten cases. The production-configuration comparison matched 32,768 stereo frames and 5,424 persisted sound bytes across both part assignments, with off-default oscillator, A REF, PWM routing, and active chorus during held edits and after reload.

The ordinary host build grows the mask symbols from 8 to 16 bytes total and keeps the position array at 64 bytes total. The upper-word test payload array is compiled only in the host probe. All mailbox shifts and mask operations use 32-bit integers. The project wire layout is unchanged. A future source-backed control in the upper word still needs its own payload position, production readback, and apply mapping. This packet adds no named control or original-control count. Native fit and timing remain outside this host qualification.

## Replay

Run from the patched isolated checkout with the combined tree's existing generated host inputs. Keep outputs in a fresh scratch directory. The red fixture is in this packet and includes the checkout's existing tests/host harness.

```sh
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1
export PYTHONDONTWRITEBYTECODE=1
# Retain the existing combined tools and full build tools in PYTHONPATH.
defs='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
export SMC_UI_TEST_FILE=tests/host/ui_superj6_mailbox_test.c
scratch=$(mktemp -d)
SJ6_MAILBOX_SCENE=b0 SMC_UI_OUT="$scratch/probe" \
    X0X_HOST_DEFS="$defs -DX0X_SJ6_MAILBOX_PROBE=1" \
    SAN_FLAGS='-fsanitize=shift -fno-sanitize-recover=shift' \
    sh tests/host/sj6-smc-ui.sh
for scene in b1 p0 p1 l0 l1 t0 t1 f0 f1; do
    SJ6_MAILBOX_SCENE=$scene "$scratch/probe/test"
done
```

For PCM comparison, compile the same host fixture in both source arms using only `X0X_HOST_DEFS="$defs"`. Set `SJ6_MAILBOX_SCENE=m0` or `m1` and give `SJ6_MAILBOX_PCM` a separate output path for each run. Use `cmp` for the matching scenes. For the red fixture, compile with `SMC_UI_TEST_FILE` pointing to its absolute path and add `-I$PWD/tests/host` to the definitions. Run with `SJ6_MAILBOX_RED` set to each of `p0 p1 l0 l1`. The baseline failure is the final A REF readback assertion.

The existing A REF suite passed 7,594 checks, including all 640 original records on both parts. PWM passed 2,074 checks and the chorus suite passed 1,034. The joined Synth One LFO and shared-row ownership fixture passed 382 checks, including authored vibrato and cross-engine history.
