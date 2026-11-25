# Same-tree Synth One AutoPan amount/rate

Add source-backed ID35/36 to the exact previous 603-control donor. Typed b21
is 0..127 amount and b22–25 exact bounded Float32 Hz with all-bits-zero as
absent. Prior Control IDs and carriers unchanged; no new physical controls
beyond the exact two AutoPan knobs. Accepted physical TEMPO SYNC updates
already-authored Hz in the same history entry. Scope is host control receiver,
not whole original audio equivalence, hardware, native fit or release.
Replay: bash replay.sh [exact-source-mod-wheels-directory].

Exact AutoPan + physical TEMPO SYNC checkpoint, **not ID64**: 635 mixed
checks covering panner stage ownership and 1,536 stereo-differing held samples
per owner, separate 80 physical Undo/Redo/readback checks, exact previous 603
reader fail-closed before writes (only b21–25 cleared in positive control).

Final second delta bpm-join.patch adds source-backed owner RATE → original
ID64. A formerly invalid b8 high bit distinguishes explicit RATE from prior
unmarked scheduler-only rows and factor-only edits. The latter do not change
tonal BPM; ordinary playback/transport changes never author it. RATE and
already-authored rate Hz share one admitted owner edit, no extra writes.
Exact prior AutoPan decoder rejects this bit before writes; clearing only b8
admits the paired staged row. Both owner orders exercised tone retiming,
live pan, Undo/Redo, stage/cold recall and actual gate interaction. S1 PART
22/23 and SJ6 19/23, each has 4 SEQ pages and exactly one FILTER K4 LINK.
Existing SJ6 duplicate LFO2 rate shortcut is identified, not altered.
