# SuperJ6 AMP note-lock opt-in engine seam (not a receiver)

The single `amp-provider.patch` applies directly to the exact
`../controls-init-only/source-s1-detune`. It includes the previous codec
and validator; do not apply the old standalone validator patch on top.
No changes to juno6, dnbx, UI, default build flags, format envelope,
flash layout, or lane capacity. No private bank inputs.

With no X0X_SJ6_NOTE_AMP_RECEIVER=1, admission and capture are v1-only
and no provider symbols are required. With the define, a real provider
must link, implementing the two functions from sj6_note_amp_provider.h.
There is no weak, no-op, or synthetic production provider. The fixture
provider is test-only, is not in the patch, and must never enter a build
source inventory.

The capture provider returns absent (emit untouched v1), present (emit
canonical v2), or unavailable (refuse update before lane publication).
The receiver supplies independently owned exact Q16 and tri-state link
from base versus effective storage as requested; do not read the ARP
sidecar, which can be stale. Absent Q16 stages do not override older cell
values. Snapshot has already been admitted and must apply atomically and
without calling the normal link-ON copy operation. NULL restores the
authoritative receiver-owned base; it must be nonfallible for an admitted
ownership lifetime. Neither callback can mutate persistent Sound or
the ARP plane, allocate, block, or reenter engine operations.

On actual note-on path: clear previous overlay before successor legacy
cell/EXTRA/MIX processing. Apply new v2 after that processing and before the
key; v1 retains its old ordinary control behavior. On unlocked note or
STOP, restore legacy base first, then clear the v2 overlay. A cold part
replacement retires an active overlay before discarding its old owner.
The engine calls no callback for an unloaded old v1 lock with no preceding
v2. This is ordering proof, not original held DSP parity or proof the
full receiver exists. In particular ID122's unresolved original branch is
not estimated or enabled here.

## Reproduce

In copied exact source and host inputs, apply the patch at zero fuzz,
copy the two fixtures to tests/host under their names, and run `run.sh`
with `enabled`, `disabled`, or `no-provider` from this retained scratch.
The third arm must fail to link with exactly the unresolved provider
symbols. Do not enable in production until the authoritative backend
has a reviewed held-transition/OSC and full control proof.

- Enabled real sequencer two-owner fixture: 100 checks. First apply precedes the
  cold key and no voice is active yet; then effect ordering through the actual step path, a held
  v2-to-v1 edge clears before v1 controls, unlocked onset and STOP clear
  after base controls. v2 captured exact authoritative Q16 values even
  when the separately persisted ARP sidecar held unrelated words; both
  explicitly unavailable and malformed receiver snapshots preserved the
  prior lock exactly. Virtual project save/reload verified.
- Disabled full project fixture: 1141 checks. v1 accepted, all 160 valid
  v2 records refused in the normal default build, also refused via
  CRC-correct project and compressed virtual-storage mount; subsequent
  save blocked with zero writes. Existing v1 layout preserved in fixture.
