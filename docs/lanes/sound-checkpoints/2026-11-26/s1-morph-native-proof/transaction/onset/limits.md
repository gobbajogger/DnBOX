The emulator's passive observer refuses projected diagnostic firmware when
its APP bytes differ from the signed package below 0x9d000. Observer attempt
under ordinary guarded replay first rejected extra timing flags; in standard
strict observer mode rejects protected package bytes. No attempt weakened
or bypassed this guard.

Separate pinned native diagnostic (not the unmodified engine / release gate):
uses the same guarded firmware project and exact assets, early original
kit commands through existing s_drum at rendered sample counter values
65536, 69632, and 73728, no external MIDI/USB/key event tape. It fixes
worker mask to the already allowed S1 A route and caps unknown stretch
lengths to 128 for both arms to make the transaction eligible. Physical
DMA is still two 256-frame calls per 512-frame half. The tap immediately
after 909 per-voice sums reads Float32 raw dry L and rev words; the later
tap is after master, before int conversion, clipping at this sample
inactive. No fitting, time shifting, or unauthorized writes.

In regular heavy replay (not this diagnostic), USB request ids and source
key event tape are identical by bytes, but first response to the 2060-ms
request occurs at 2325/2299 ms (Elite/Pro compression-only) vs 2359/2347 ms
(stack) and 2324 ms (forced-serial Elite shadow). These are vUSB response
times, NOT event consumption timestamps. The physical key FIFO and
command queue are drained at 256-frame engine blocks; the physical input
tape is on ms, the rendering/d8_frame counter is on frames. Current source
stack vs control first PCM difference at 70,208 frames is reproducible, and
exact offset is never used as a fitted alignment.

In sample fixed native diagnostic both boards receive the same authored test
hits in the existing firmware handler at the exact same sample and same gain.
The pre-909 dry L tap remains 5 ULP different between arms on both, with
zero rev tap; post-master differs 7 or 8 signed-24 units depending on board.
Thus physical event ingress time alone cannot explain every difference.
There is also an actual number-of-rendered-frames mismatch (512/1536 on
Elite/Pro), consistent with the stack pin test's 1/3 late DMA halves. We
therefore do not assume all unused slots start with identical live state.
Scope is a single sampled pre/post boundary, not all-instrument parity.
All stock safeguards intact and byte-identical selected disposable NOR before/after each shot.

### Serial-commit control

In a third same-fixed-sample Elite arm (advanced source shadow compares output
but always mixes the Core0 source), exactly 152 published worker state and
raw voice comparisons all passed; no unavailable shadow and no state/PCM
mismatch. Despite this, whole output diverged at the first control onset
(frame 66115); serial-commit shadow starts at frame 68160, renders 6656 fewer
frames, and reports 13 late DMA halves. The forced-serial pre-909 tap at chip
sample 73728 is 1059294357 instead of 1059294313 (44 ULP); post master at
same tap sample is 336820 vs 337123. This establishes the mistimed onset and
pre-sum difference do not require propagation of an incorrect published
worker voice or unpinned external hit arrival. It does not establish which
other owner or prior render subdivision becomes the first live-state
difference. Instrumentation serially does extra work and is not a timing score.
