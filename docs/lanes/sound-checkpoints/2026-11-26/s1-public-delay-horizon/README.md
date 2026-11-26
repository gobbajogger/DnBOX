# Synth One exact public-front-end delay horizon, host only

Original AudioKit S1DSPKernel.hpp:399 exposes ID43 min 0.0003628117914,
max 2.5 seconds. Setter clamps factory/preset/kernel/AudioUnit values;
dependent knobs and synchronized direct time also call that clamp.
S1DSPKernel+process.mm:235–236 doubles time for first two long rings,
preserves t for remaining two. Old numerical phase remains 10 seconds
in the exact host donor independent of actual physical storage.

Current compact tree s1c_front.c uses the same bounds and re-snap on
rate/tempo sync; s1c_delay_time_fit rejects >2.5. UI ui.c:1681 uses
s1_delay_time_choose (original_presets.inc:1295), which resolves codes
1..128 free/129..143 sync through the same frontend. Project load, typed
saved code, undo, and reapply call the same helper
(original_presets.inc:1283/1322/1326). Factory S1PACK command s1c_part.c:
34,78–92 emits through the clamp and saves clock_factory_delay; direct
clock restore :332 copies those bounded words. Low-level s1c_set/sidecar
are not application raw validators. The decoded 906 values are inside the exact binary32 range; two raw
values equal the exact rounded minimum (see outside.log). Actual staged
remote adapter remote_s1_panel.inc maps stable ID43 to R_S1DLYTIME,
the same pref_set path noted above, not an arbitrary raw float write.
The current public transport adaptation is source-inspected, not a
separate browser or firmware fit gate.

Ingress probe compiles the actual frontend and observes 14,630 valid
writes across 11 BPM inputs in both modes, and rejects 2,464 invalid
stored codes. The observation stub supplies only bounded-arena admission
and captures exact writes; this is not target fit. The exact source
comparator tested 3,684,000 stereo frames with bitwise equality,
covering 2.5s exactly, the actual positive public minimum, held ramps
across range, feedback up to .982, fade and off/on, explicit resets and
both logical and physical wraps at 44.1 and 48k. 43 latest whole wet
holdouts incl Pad, Organ, Hall, Deeper and bank positions are byte-exact
against the same codec donor, same note tape and authored controls.
The negative sub-minimum C sweep in debug.log produces real differences:
closely spaced cubic interpolator coordinates can read differently aliased
old/future physical slots, so this proof is NOT a raw 0-second support
claim. Zero wet remains independent and can be faded.

Measured capacities including +8 interpolation guard per history:
7,056,016 B → 2,646,136 B @44.1k and 7,680,016→2,880,144 B @48k.
This removes unused fixed-horizon allocation; it does not compress
actual active history. The active effective-tap budget still needs
an independent older high-band encoding, not simply a fallback to
less treble when tap age exceeds a short recent window.

budget.jsonl: favourable 3 folded histories (not Float32 exact due to
rounding), two long 2t and one short t, 32ms recent detail at 4bits
costs ~2,118 B before metadata. A 4,534 B/owner optimistic total leaves
long low-band capacity roughly 4.5k samples/s at Pad .2162 s (~2.24 kHz
Nyquist before guard), far lower at Hall .5616 s or public max.
Even independent 8x long-low and full-rate recent-high can meet tenfold
vs exact Pad tap on paper, but no recent detail supplies its 216/432ms
aged taps, where the actual Pad has substantial 5–16kHz tail content
(-36.47/-30.75 dBFS). Optional future experiment: budget an independent
dated high residual retained by signal-space criterion, not fixed
carrier or simple resample, kept in original chronological feedback
and short-right crossfeed order. Must keep real active gains and phase
under impulse, held Pad/Hall/Deeper and worst retime, then model-driven
bank/target timing only if justified. At current certified budget no
fully supported split design has been proved; existing pure downsample,
short lock and narrow adaptive carriers remain rejected.
