# SuperJ6 independent Delay: typed project/Undo host witness

Replay against exact composed sj6-mod-pb-typed-join POSTIMAGE (source manifest
b71f366f… plus prerequisite patches). Apply **only** delay-typed.patch to that
base, not the separate optional delay-25-26 patch. Pre/post hashes pin all
seven touched files. Opt-in X0X_HOST + X0X_SJ6_DELAY_WITNESS on the disposable
DNBOX_137_HOST_WITNESS typed fixture. Not a product format assignment.

Delay Enable/Time/Feedback/Mix occupy bytes 16/17/18/19 in each distinct
canonical first owner row, T_303 rows 8 and 10; each block ends before the
32-byte row boundary. Juno MOD is b14 and PB b10/11; S1 MOD b3 and S1 PB
b4/5; untouched old typed canonical path rejects any remaining nonzero byte.
Both Delay blocks allow only 0/1, 0..127, 0..127, 0..127 respectively.
Access requires exact typed Init owner; no Delay UI in regular legacy or
factory contexts. Values can remain inert in a typed row for other engines;
they are not applied or edited there. No extra format bytes/lock channels,
and no per-note Delay semantics or shipping migration. The existing full
Sound/stream tuple digest binds every byte, so typed recovery remains
read-only rather than cherry-picking the newest Delay words.

K1–K4 exact physical page navigation/edit, whole Sound diff (zeroing only
b16–19 reproduces baseline), engine setter and separately owned delay lines,
real HOME/REC and HOME/PLAY Undo/Redo, portable stage validation, true native
virtual-object save/reopen, and torn Sound/stream rejection with read-only
recovery all exercised. Exact predecessor validates the prior shape with
the four fields manually cleared, but refuses populated media both at stage
validation and native virtual store with zero writes (also blocks save).
No settings/protected storage or real-device tests. Recovered exact projects
render, but reject edits and writes. Undo publishes persisted words, audio
takes them at the next normal prepare boundary. This proved full raw recall
for the bounded always representable free-time profile. Authenticated sync
detents/tempo admission remain separately tested in delay-25-26; this typed
fixture neither adds nor persists an independent sync-mode control. It does
not imply original wet-graph timing/PCM equivalence.

measure.py compiles the same installed experimental DSP receiver and alters
only cell get/put. With the isolated left 0.319117 impulse, it
drives actual raw controls, demands sample-exact dry while history writes,
opens wet onto retained history (1.5 s), checks a second fractional-time
impulse at raw Time=1, Feedback=103, Mix=127, plus all-zero right channel.
See numeric receipt. Smallest tested (packed 4-bit companded) creates wrong
wet output (negative SNR); 8-bit and 16-bit also lossy. Memory is for ONE owner
and stereo at 44.1 kHz with exact 66151 cells/channel, not including metadata.
Double the data for two simultaneous owners. No general lossless compression
bound, native timing, native capacity, or recommended production storage
inferred; no MIX ring sharing. No native compile, product/device write,
Git, or publication.
