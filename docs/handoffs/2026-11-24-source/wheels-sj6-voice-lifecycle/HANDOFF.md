# SuperJ6 VOICE held/pedal/panic lifecycle, same host source

Apply voice-lifecycle.patch to exact b36–37 VOICE compositor (existing
893+VOICE fixture source). It does not touch carrier bytes, storage format,
original_runtime.c or add any control. VOICE K1/K2/K3 still Mono, Legato,
Glide; K4 empty. Host only, no device or native fit. For a translated
target take only independently checked live behavior, not a test-only layout.

## Build9 admitted evidence

Original AU static SHA256 a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1.
Mono callback 0x1000f14f0 compares requested state to 0 and uses the actual
normal-stop traversal 0x1002b1ff8; the post-set non-Saga path includes
0x1000f1708/0x1000f1734. Swift AudioKit AKSampler class 0x1007a7428 +224
resolves the virtual slot used by 0x1003db5a0 to 0x100545134, passing zero
for the immediate-kill flag. It is an ordinary MIDI-note-by-note release,
not hard all-off/retrigger or a phase reset. Current compact owner cuts
its scoped provenance/stale-off queue and soft-releases existing gates.
Release tails and peer synth aren't reset; old physical releases don't
cut a later distinct pitch. Untranslated Saga/private sampler branches
are not asserted to have byte-identical side effects.

SustainPedal selector 0x1005452d4 -> 0x10037d170 -> 0x10037029c.
Pedal down marks held; note-off's 0x10038e4a0 decrement returns no
release while down. Pedal up iterates deferred physically-up entries
via 0x10038e4f4 then calls ordinary release. Current-pitch fallback uses
0x10038e544, lowest still-held note. New direct-on prior physical held
predicate 0x10038e51c excludes a pedal-only voice, so no fake Legato and
no inherited glide on a fresh phrase. Tests go through existing DnBOX CC64
(boolean selected at the existing threshold); no claim on other AU MIDI
mapping layers. The DnBOX ingress coalesces duplicate holds, so backend
uses a 128-bit effective pressed set, not a duplicate counter. This avoids
a stale gate after only the final aggregate off is dispatched.

Original actual rate change parameter 9 stores rate through 0x100371290.
The render branch at 0x1003a5c20–38 skips advancing offset if rate zero;
the pitch composition at 0x1003a5e2c still consumes the offset. Thus zero
Glide Time pauses a currently active offset; positive resumes it and a
fresh note (and true panic) resets it. Reference for the existing hard
all-voices stop: AKSampler stopAllVoices 0x100544318 -> 0x10037d158 ->
0x100370328 clears note tracking and calls voice invalidation. Host uses
the existing DnBOX FX+SEQ panic ingress, not an invented CC123 alias.
No extra release/velocity behavior attached to unverified original modes.

## Tests and limitations

The two-owner real UI/audio fixture exercises both directions of held
Mono, soft tails, untouched peer, direct MIDI duplicates, CC64 deferred
fallback and new pedal-only phrase, live zero-rate pause/resume, and
actual panic chord/next fresh attack while retaining b36–37. Re-recorded
b36–37 history/phase/cold-recall test confirms active undo uses identical
soft cut before restore, no actual writes from history. No original AU
execution, hardware, output PCM identity across engines or native slot.

Replay: bash replay.sh [exact-source-mod-wheels-directory].
