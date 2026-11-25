# Synth One Delay Input Cutoff tracking (original ID137), typed b39

Apply at the exact 893 host-combined controls + 1539fb3a… read-only native
carrier stage. Enable X0X_S1_DELAY_TRACK_TEST=1 alongside that trial's
existing flags; don't define X0X_HOST for actual native builds. The
ordinary shipping/default path does not take ownership, expose a knob
or relax validators. There is no default migration or device write.

The original visible knob is in Synth One's Developer (HAQ) panel.
Proof and upstream pin are in ../s1-delay-input-cutoff-137; existing
compact live P(DICT) responds in the Delay input low-pass graph only,
with existing warm history and the existing control-rate coefficient
implementation. Prior independent 1,121,337 grid proof bounds the
cutoff mapping difference at .2265625 Hz on host (not original ladder
PCM or a full-rate assertion); no DSP, delay ring layout, fallback,
gate, LINK or MIX change in this delta.

One exact independent physical control is Synth One DELAY K2 IN CUTOFF,
50–100% at 1% host detents, default 75%. Existing Feedback K1 retained
when enabled; otherwise K1 is empty, not aliased to another parameter.
The typed field is only second-row index 7 (owner rows 9 and 11), b39;
no pointer arithmetic across C arrays, no other typed bytes assigned.
Canonical code 0 = default/unmodified 75; codes 1…25 = 50…74, 26…50 =
76…100. Anything else fails before staged adoption. Forged owner bytes
fail; switching to a different synth makes b39 dormant, preserving its
owner's exact previous byte but hiding/rejecting its active ingress.
On returning to S1 Init, the corresponding owner is restored. It is
base-owned, not added to note-lock capture; pending preset, live note
lock, busy operation or read-only recovery refuse physical changes.
Undo/Redo uses existing same-layout snapshots and checked held-audio
restore, not store operations or queued unrelated targets.

224 real UBSAN host checks: on two owners, real SMK DELAY K2 edges
and readback, physical HOME+REC/PLAY Undo/Redo, only both b39 bytes
changed, actual warm LP coefficient moves in expected directions and
all existing ring bytes unchanged at the edit. Full virtual save,
staged/portable reload and wrong-engine checks. Independent host trial
rehearsal with Feedback disabled demonstrates both RAM edits and
ordinary save refusal on original written object bytes. Exact prior
carrier does not know b39 and refuses real native-store and portable
media, plus ordinary/device-BT write paths; clearing just the typed
field permits read. No-X0X_HOST GCC syntax smoke is not XIP/POOL/boot,
native linker, device, or timing qualification. No presets touched.

Next real independent Delay control is original ID43 Time (not ID44 MIX,
ID41 ON or dead ID138). The setter already applies source free vs snapped
beat divisions. However the current cold partition fixes decimation and
ring spans; a warm larger target can silently modulo-wrap. A scoped next
test can preview the accepted target, then admit only if all line taps
are strictly within current history (for current H16 graph: 2T<L0,
2T<L1 if stereo, T<L3, with interpolation guard). It must not promise the
full original range: warming a longer valid-time ring after smaller
history has been overwritten cannot preserve lost history. Never repartition
and cut tails, alter untouched allocations, or disguise refusal as a
valid original setting. No ID43 source or wire edits in this packet.

Replay: bash replay.sh [exact source-mod-wheels path].
