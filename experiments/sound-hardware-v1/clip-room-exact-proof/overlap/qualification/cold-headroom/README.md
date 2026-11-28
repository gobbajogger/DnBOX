# Cold headroom experiment against the 662013f8 checkpoint

Copied the assembled diagnostic source before editing. Baseline proper narrow lazy-peer build reproduced the 4,980 B Pro app budget; flags and configuration unchanged (see paired logs). No render algorithm, stack guard, runtime reserve, sample, audio buffer, or display coverage changes were made. Both patches are optional and not release-qualified.

| Native case | Elite app XIP free | Pro app XIP free | Extra Pro vs matched baseline |
| --- | ---: | ---: | ---: |
| Baseline (Elite preceding exact checkpoint) | 4,972 B | 4,980 B | 0 |
| Cold UI annotation only | not run | 5,012 B | +32 B |
| Cold storage exact shared selector only | not run | 5,028 B | +48 B |
| Both source patches | 5,068 B | 5,076 B | +96 B |
| PI32 linker --relax, independent | not run | 4,980 B | 0, byte-identical app |

All links leave 27,392 B pool above 8,192 B reserve, .data+.bss 96,532 B of 98,304 B. Code RAM previously ends at 0x01c0111c, exactly 1,764 B until the 0x01c01800 Bluetooth allocation. The pre-existing ladder-RAM experiment ends at 0x01c011c4, leaving 1,596 B. Ordinary BSS free is 1,772 B but cannot be used for code by moving static storage without changing guarded execution mapping. No BLE guard change suggested.

Storage selection fixture compares both C versions across 362,880 cases (header/body outcomes, lengths, both orderings/ties, every I/O injection through the largest body, save vs load), identical output/trace hash 2c126e23fe4fef53. This preserves pre-existing exact I/O failure semantics rather than changing maintenance policy. Existing storage scratch suite (324,069 checks, full NOR match, 441 saves/loads each) and project stream copy (274 chunk cases, 15 projects) passed. Compiler UI attributes do not touch control flow or image pixels at source level; native real-time UI/Audio replay not performed, as the actual gain misses the several-KiB target.

The next biggest app tables are audio data and therefore out of scope of this lane. The fixed external XDATA and FDATA regions leave 494 B and 232 B respectively; the 2,692 B FITTAB gap does not hold an application fallback in mismatched-package scenarios. No blind linker or package layout changes retained.
