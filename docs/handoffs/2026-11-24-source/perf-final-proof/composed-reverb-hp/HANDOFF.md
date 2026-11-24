# Synth One Reverb HP (independent of rejected Delay Feedback)

Source-only host-qualified batch based on exact product post-MIX;
PREIMAGE/POSTIMAGE identify all four changed files. Nothing from the
rejected Delay Feedback DSP or control is included. No native fit,
emulator, hardware, or release verification.

Original upstream pinned closure S1Parameter.h: ID 39; Kernel
S1DSPKernel.hpp: 80–900, default 700; S1DSPKernel+process.mm assigns
that exact physical cutoff to the two Butterworth high-pass inputs
of the original wet reverb. Compact production already consumes
P(RVHP) in its bilateral hp2 reverb input branch. K4 on EFFECT now
edits the original ID, with real Hz formatting; no new filtering,
retuning, allocation, delay-layout changes, or claim of overall
original reverb equivalence. One factory float is saved per S1
owner to restore exact authored program targets after a note lock.

Same 15-byte owner/lock record. Legacy high=31 is impossible in its
148²×75 LFO tuple (the smallest positive ordinal there is 1,966,051);
new exact byte1 f8 plus byte0 one-based stop is fail-closed in older
readers. Both former MIX bytes layout and older OSC/LFO formats
continue unchanged; promotion occurs only when this control is set,
with the same stored byte-claim validation before admission and commit.

The exact prior product red fixture lacks ID39 editing. New focused
host passed 1,696 checks: both physical K4 owners, actual held wet
4096/4096 affected and bypass 0/4096 affected, untouched peer stereo
PCM exact, both wet enable flags and full live pool/line size array
stable. First detents moved from 900 to 893.543335 and 219.399994 to
228.503937 Hz, following the displayed exact factory position with no
change of transfer or cap. Save/reload, note-lock stored capture,
live lock and unlocked onsets, stop rollback, and no writes on busy/
invalid/opaque admission are exercised. Both physical legacy MIX
controls still passed 1,100 checks. The exact pre-Reverb-HP reader
passed 257 checks: accepts old control, refuses new Sound and lock,
and does not touch stored six-object byte images. INIT and all 694
factory programs matched byte-for-byte (1,024 stereo frames each,
same SHA2 2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f).
Actual LCD screenshots show the shared EFFECT card at 700 and 80 Hz.

Repeat with bash replay-host.sh /path/to/exact/post-MIX /path/to/pinned/s1.pack.
The pack is read locally and hash-checked; no pack or PCM is included.

Auto Pan Amount ID 35 is not substituted: the compact delay uses
PANA/PHMIX to select a shared-vs-stereo line layout on repartition.
An already-rendering shared delay cannot become a true stereo delay
from the Amount knob without migration or clearing history. Delay
Time and Feedback remain independently held for documented transfer
and topology reasons.
