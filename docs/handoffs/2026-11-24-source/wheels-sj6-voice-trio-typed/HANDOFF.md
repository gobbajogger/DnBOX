# SuperJ6 physical Mono/Legato/Glide Time, same-tree host proof

Apply voice-trio.patch to exact 813 (RANDOM2 not needed) or current 893;
source pre/post manifests and host fixture runners identify both. VOICE
K1 Mono, K2 Legato, K3 Glide Time, K4 empty. Mono actually selects voice0;
Legato uses held-pitch retune without resetting envelope/captured velocity;
original K1 Mono arms actual Glide rather than adding a fake enable control.
Glide time is 0–500 ms; active signed semitone correction advances the real
oscillator and sub phase and preserves the remaining correction across
held phrase note changes. Original evidence and exact limits are in
BUILD9-VOICE.md.

Canonical owner tuple is in the second typed row: b36=0 only if both flags
and b37 are all zero; otherwise b36=0x80+(owner<<2)+Mono+2*Legato,
b37=0..127 normalized original Glide detent. J60 b32–35 untouched; no other
allocated field or owner index reinterpreted. Both owner tags and malformed
rate rejected. Same-owner retained mode for future Juno selection is legal,
but no application in S1; a copied wrong-owner tuple is rejected at admission.

Physical direct authoring, real Legato and Glide audio, before-write undo
refusal on held mode transition, idle undo/redo, cold recall, and original
prior-reader refusal before any write. Canonical absent tuple preserves
ordinary defaults. Exact source-only host witness, not native memory, device,
preset/lock format, or release. Active Mono toggles and original pedal
behavior remain deliberately scoped; no velocity/engine audio parity claim.
Replay: bash replay.sh [exact-source-mod-wheels-directory].
