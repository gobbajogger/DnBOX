# Voice/Output exact coarse rebase

This is the unchanged combined Voice/Output production patch applied in
a private copy of the canonical post-held/ID73/authored-coarse source.
The full before manifest SHA2 is 5be84b92b4d329b4ed77708b3859dc01db5d960b9b3cfc71e2fdea51420e022b;
after is 0a4dcfe87602b2a56ea3ad503d4a3f9e6cf83680f4f5834b58152e56b5916a0b.
No production code beyond the exact patch SHA2
f86a744c6355ab0b4394927b7bf76e331d1c2296b702578a865060564e3c9d2a.
Apply receipt reports line offsets only, not dropped or fuzzy hunks.

Two independent original controls: ID120 VOICE/MONO LEGATO and ID32
part OUTPUT/PART LEVEL. Same-note history, exact mono pitch/retrigger
trace, no change in poly, wet/dry and both owners, both lock states,
and exact no-write old project/lock gates. Prior HP-only old accepted
fixture exported before adding new Level, intentionally distinct
from real new combined state. New Level uses exact trusted cell byte3
with dedicated edit flag. Legato uses reserved packed tail digits;
both coexist with earlier Widen/HP/LFO. Host-only fixture instrumentation
is absent from product patch. The root tree was never modified.

Standalone replay: bash replay-host.sh SOURCE AUTHENTICATED_S1_PACK.
The script verifies the pinned full source before copying, then verifies
the exact joined file hashes, tests real old/new readers and host
audio, records actual LCDs and compares INIT+694 unedited audio.
Source/cross-reader tests are not target memory, hardware, or release
qualification.
