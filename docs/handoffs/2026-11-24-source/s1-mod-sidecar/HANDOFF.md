# Synth One MOD routing: host-only same-layout carriage

Exact b71f366f source-mod-wheels plus the independently pinned ID137 joined
host parser and S1 MOD rate receiver. Reproduce with
`sh replay.sh ../controls-init-only/source-mod-wheels` from this directory.
This is not a native build, preset import, user UI, or shipping format.

Original source 6466a371: WheelSettingsViewController selects Cut-off, LFO1
Rate, LFO2 Rate, and Manager+ModWheelDelegate sets selected sound routing,
refreshing the visible position without forwarding the refresh as a gesture.
The explicit host selector changes only persisted owner byte 3 and selected
receiver. No synthesized movement, no wheel-induced save, no retarget on
visible part change. A new real ADC gesture update while the wheel is held
uses the new route on the captured owner; release re-arms normal ownership.
The alternate rates use the already verified original receiver mapping.

Shared typed format is not forked. Owner sidecar's byte 3 encodes 0/1/2,
with 3+ rejected and the rest still canonical. ID137 remains untouched.
New note version 3 is Synth One Init only; same lane size, reserved lock-local
tag byte alone stores the selection, restores zero in the copied record before
running the unchanged note tag validator. No mutable-base lookup while
locked. Actual onset selects the locked route and the next unlocked note
returns to same owner's live base. Switch/restore and real undo/redo apply
the route without synthesizing a wheel event; undo during a real held wheel
is refused, not queued through the test. Existing old projects remain
unchanged and cannot opt in by having zero bytes. No link/page modifications,
no guessing a physical control role. No native cap or default enable claim.

Old refusal uses prior unchanged product and older ED_PROJECT routine on the
actual new-writer bytes, foreign setting/sound mixed media, and virtual USB/BT
side effects counters, not physical hardware. Same-typed torn transaction is
still inherited RED in the underlying parser; mark as test-only even with all
observed carriage checks green. Until transaction + native/UI acceptance:
do not enable the shared format by default, migrate projects or ship.
