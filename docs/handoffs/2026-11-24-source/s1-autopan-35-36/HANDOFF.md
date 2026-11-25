# Original Synth One AutoPan Amount/Rate (35/36)

Pinned AudioKit/AudioKitSynthOne 6466a3715c96b7ecf1dd255a218cbd571408a314:
EffectsPanelController binds amount and normalized rate; source process places
mono→equal-power stereo before phaser, delay, reverb, and derives rate via
original 0.4 taper and 19 synchronized divisions with a separate 10 Hz max.
Actual compiled Soundpipe osc/pan2 from the admitted upstream fixture serve
as oracle. One new authored-only Q24 phase step reuses the existing pan
phase and mono→stereo stage. No new bus, compressor, wet buffers, or phase
reset. Coefficients retain the compact sine interpolation: measured worst
gain error 8.27e-5 and gain power error 1.51e-4 vs pinned Soundpipe;
phase words and side signs exact across active changes. 18,432 contiguous
samples/owner, including 4,608 rendered wet-tail samples with exact zero
mono panner ingress. This is a measured host comparison, not entire
original AudioKit PCM, native fit or timing.

Physical AUTO PAN Amount and Rate two knobs coexist next to current FX
controls; untouched legacy has no change. Fresh typed b21=Amount q7 exact
0..127 and canonical b22–25=little-endian Float32 applied Hz, +0 all-bits-zero
absent, valid positive original range only. Physical encoder edits have
original normalized free or bounded sync law, while retained data stores
actual Hz, not synthetic choice codes. Accepted existing SYNC tone edit
updates both mode-dependent runtime and already-authored current rate in
its same history change; none of this creates a second write. Read-only
fallback edit/undo guarded and ordinary legacy media untouched. When
authored data is all absent, runtime retains the previous compact Init
value and PCM, including its stock tempo-sync dependent frequency close
to .2777778 instead of falsely claiming the pre-sync preset constant .25.

Tests: 200,573 real DSP/original module/rate checks, 80 physical
UI/owner/save/undo/Hz-validation checks, 4096 untouched stereo frames exact;
full strict no-write old USB/BT refusal using actual writer media.
Replay from this directory: sh replay.sh ../controls-init-only/source-mod-wheels
No source redistribution of oracle modules; hashes reference admitted local
upstream inputs. No preset, native, device, Git or release changes.

Integration boundary: existing typed owner ARP BPM (b7–9) in the wider
candidate currently feeds only separate source scheduler. Its single actual
admitted BPM control will need s1c_front_set(64) to rebase all dependent tone
rates and refresh b22–25 within that accepted transaction (not MIDI/shared
transport playback or note-lock callbacks). The independent small proof
here validates direct real DSP tempo and existing physical SYNC, does not
assert that wider BPM control has been composed. Rebase overlap only typed
canonical header, common UI undo/enum and original_presets apply points;
AutoPan should remain playback-write-free.
