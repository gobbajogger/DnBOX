# Synth One control census (source + preexisting evidence)

Read controls.json as the authoritative machine ledger. It includes all 150
stable original kernel IDs once, plus seven entries outside the enum. Each
stable row carries the original symbol and line anchors, separate stock and
candidate panel bindings, compact runtime sources, exact authenticated
ordered-command occurrence count, and an owner/persistence/factory proof-set
key. A factory command is not automatically an active parameter, and a
compact enum reference is not an audio parity result. No original iOS UI
denominator is established.

- 14 direct stock editors (13 typed slots and standalone Crush Rate).
- 11 additional compact editors are host-qualified only: nine oscillator,
  sub, FM and noise; two filter mode/tracking. Those checkpoints verify both
  bass owners, note locks and reloads, untouched 694 + Init PCM nonregression,
  and scoped exact factory edit/restore. Not an indication that they are
  rolled into the selected final integrated/native build.
- Five IDs assigned to LFO Work: shape/rate 13/15, 45/47 and shared sync 139.
  The prep covers original rate/shape/sync and storage capacity, not an
  accepted live/recall implementation; don't count those five yet.
- 55 shared ARP analogues (seven global and 48 step-data fields). Not 55
  knobs, no proved original scheduler or factory import equivalence.
- Three explicitly unused/deprecated (18, 33, 145), one performance-only
  (140) and 61 not exposed. The last number is an engineering ledger of DSP
  or app identities, not a count of missing iOS controls. LFO and sequencing
  scope are kept separate. DnBOX-only FILTER LINK is an outside-enum tested
  patch, and does not establish the original app has that switch.
- 144 IDs issued for each of all 694 authenticated build recipes. Excluded
  per-program command IDs: 18, 31, 33, 140, 144, 145. Values present in the
  remaining columns may still be zero-only, not consumed, or shared/disabled.
  See each ledger row for counts.

## Batches to assign, after exact pin reconciliation

1. **Finish oscillator voicing** (3, 4, 30, 31, 146). Add pitch/detune and
   band-limit exposure in four-knob pages while proving semitone bounds,
   7-bit edit-to-original-domain law, both owners, factory/fixed-bend
   interaction and typed save/lock/reset. ID 31 is not a compiled factory
   command; don't fabricate an authored value. Include pure oscillator
   render proofs and distinguish compact approximation from original law.
2. **Independent wet FX and output**, no timing/FX estimator changes without
   owning those tickets. IDs 35–44, 116–119, 121, 137–138; then output volume
   32 and all 15 compressor controls 122–136 in separate rows. Reuse verified
   compact behaviors where present. ID 138 is not actually wired in compact:
   original setup binds it, compact passes zero resonance to the delay input
   ladder. Don't falsely expose the global MIX master as the synth's master
   or reverb compressors. Verify edits change intended owner and stage,
   wet tails/on-off, project/note-lock restore, and no untouched PCM change.
3. **Voice/tuning** 20, 21, 120, 141–144, 148. Separate mono/legato/glide from
   pitch/ranges and A4. Compact bends a fixed ±2 semitones through the
   wrapper and its voice uses fixed note_hz, although the frontend stores A4.
   Implement real key-frequency and bend endpoint readback before claiming
   original reachability. Keep the original app's tuning-set workflow and
   mod wheel route (0 cutoff, 1/2 LFO rate) distinct from DSP ID fields.
4. **After LFO Work handoff**, batch untouched amplitudes 14/46 and original
   destination selectors 48–59. Review code for each target and readback
   rather than exposing ordinal guesses. Include tap/cutoff etc only under
   authentic original mappings. Compare mono/poly owner and active preset;
   restore dependent targets on note-lock exit; do not duplicate 13/15/45/47/139.
5. **Independent ARP/SEQ parity** 60–114 plus 147: match interpreted native events
   to the original rate/order/tempo multiplier and three step lanes. Preserve
   shared pads, no cuts. Exact authored preset/program transitions, pitch,
   octave and on/off all need proof; a gate-length knob is not original step
   on/off. No shared tempo-sync wiring may silently override patch BPM.

All new storage must use proven unclaimed typed extents and fail closed on
legacy/unknown records. The existing FILTER MODE note-lock record is already
13 of a 15-byte tail, and LFO Work has an explicitly competing small-footprint
model for remaining bits/bytes; coordinate before allocating. Do not count a
descriptor-only exposure as complete without physical detents, both owner
isolation, audible/semantic proof, project recall/locked-note separation,
factory replacement and final native XIP/timing evidence.

Regenerate: python s1-control-census/generate.py. No checkout, build, native
test run, branch write or publication was done for this census.
