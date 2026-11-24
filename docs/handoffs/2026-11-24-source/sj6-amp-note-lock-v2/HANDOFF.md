# SuperJ6 note-lock AMP state: storage component only

Base: `perf-final-proof/controls-init-only/source-s1-coarse`. No device, bank
data, product tree, Git or release writes. The proposed validator must NOT enter
production standalone: it would admit v2 without playing the snapshot.

## Record

Existing 72-byte lane and 64-byte val. Keep cell [0..31], MIX [44..48],
EXTRA [49..63] with current validators. Juno-only lane.i=2; header target,
step keys, used and rsv validation are unchanged. Replace derived 12-byte
tag with four little-endian Q16 words [32..39], [40] low four bits present,
high nibble 0 absent/1 explicit OFF/2 explicit ON, [41] canonical
BASS_SOUND_EDITED flag recomputed from cell, [42] zero, [43] 0xA2.
Zero-mask/no-link is not a v2 record; inactive stage words must be zero.
v1 tag decoder requires its byte 11 to be zero, so relabeling only lane.i
cannot turn either encoding into the other.

The v2 reader validates the Juno cell directly (never legacy-adopts), then
reconstructs its canonical engine/schema/count/Known/factory/Edited tag
using the existing registry and encode/decode check. Canonical Edited is
checked redundantly. No v2 state is inferred from an invalid cell. A valid
Juno identity has no independent retained information here: the v1 tag
identifies schema, count, preset and edited state already checked against
the cell, and is not an integrity hash. Actual enclosing project CRC remains
required. The per-step duplicate check stays independent of lane version.

## Integration seam (not implemented)

1. Snapshot into sj6_note_amp_t from an authoritative receiver-owned snapshot
   with exact Q16 values and independent ID115 presence. Existing ARP sidecar
   may be stale after the shared primary editor changes audio; do not use it
   blindly, do not invert Float to recover Q16, and do not conflate 115 and 116.
2. Emit v2 in the existing capture local lane only for a nonempty accepted
   snapshot. Replace the tag with sj6_note_amp_encode, set i=2 and validate
   before publishing used=1. No extra lane or pool bytes. Encoded absent values
   use v1's behavior, not a made-up forced value.
3. Existing pre-note playback consumes cell/MIX/EXTRA, not the v1 tag.
   After existing Juno patch, before key-on, apply the receiver snapshot
   atomically and restore previous overlay at v1/unlocked and STOP. Do not use
   toggle-ON as restore: the original edit copies primary to secondary and
   would overwrite independent stored values. Runtime must define held edit
   and same-note ownership, fail without partially mutating, and leave the
   peer and idle/legacy PCM unchanged. No assertion of those behaviors here.

## Replay

`bash run.sh prior` and `bash run.sh new` use ordinary full in-memory host
engine/project fixture with real capture, reader and saver and the specified
ABI. The writer-side v2 overlay is injected by the fixture, not by a real
receiver. The prior arm receives exact CRC X0XP payloads and an injected
compressed stream via virtual storage, not a mocked validator. The new arm
validates and reopens the full dense project. Equal logical lane size does NOT
imply equal compressed disk size: measure_project remains an admission gate
before any settings/sound/pattern writes. This dense stock-pattern fixture is
not a proof every pre-existing near-capacity project can add v2 locks.
