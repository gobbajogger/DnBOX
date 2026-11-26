# SuperJ6 short Classic history E4M11, no-copy carrier
Only changes the 32-bit binary word carrier in the canonical E4M11 codec,
from memcpy to same-union reinterpretation in compiler C mode. Explicit
binary32 assertions and both guarded PI32v2 linkage/disassembly receipts
pin the ABI. Signed and quiet semantics, rounding ties, nonfinite policy,
ring bytes, center, gain and wet tap arithmetic unchanged.

Host matched all 65,536 representable words, adjacent ties, 514,336 extra
words and canonical text result for all eight saved typed-cohort wet and
low-tail metrics, with no normalization. Native code includes no memcpy
call in codec, Classic tap or caller. Both owner and part sizes unchanged,
and +564 pool stays. Linked APP overhead +144 bytes vs Q15, 32 B smaller
than canonical memcpy (remaining Elite 2828 B, Pro 2840 B).

On actual existing dual-owner 256-frame Classic fixture at 360 MHz,
union timings are about 747–749 us, much faster than memcpy (1629–1689 us),
but +208–214 us against Q15 534–540 us. All short cue word digests equal
canonical E4M11 on both diagnostics; no virtual NOR writes. No proven
whole-scene margin, so do not adopt as release fit evidence; same starter
profile and guarded unmodified native donor constraints apply. This result
does not validate original app sound parity or long delay.

Donor files pinned in donor.sha256. Replay source-only host and paired
diagnostics with native.sh/replay.sh and existing extract.sh. Includes no
input/output PCM, preset assets, firmware containers or executable.
