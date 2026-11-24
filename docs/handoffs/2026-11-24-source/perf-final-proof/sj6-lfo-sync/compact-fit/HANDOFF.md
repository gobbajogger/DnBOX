# Exact SuperJ6 LFO sync on the compact PWM2 fit arm

| Actual linked arm | Stock Elite / Pro app free | Guarded Elite / Pro app free | Companion | POOL above reserve |
|---|---:|---:|---:|---:|
| PWM2 + compact pages reference | −12,184 / −12,176 | +1,116 / +1,124 | 13,662 B | 512 B each |
| Sync + compact pages | **−13,148 / −13,140** | **+188 / +196** | 13,698 B | **512 B each** |
| Sync + compact pages + optional owner check | **−13,100 / −13,092** | **+252 / +260** | 13,698 B | **512 B each** |

Exact LFO sync growth vs compact PWM2 is 964 B stock, 928 B guarded, plus 36 B guarded assets. Optional engine owner check saves 48 B on this stock join and 64 B guarded; its prior normal 64 B does not carry forward. All final maps retain native POOL allocation 335,360 B and 512 B beyond 8,192 reserve. RAM text 2,748 B, core1 text 420 B, XDATA 7,775 B remain unchanged. Host struct growth +16 does not predict linked native section usage. The unchanged normal production region still does not link. The disposable lab companion is not authorized stock fit: its address physically aliases user storage.

## Target compiler and parity

The upstream Work patch emitted unavailable target libc __fixunssfdi and __fpclassifyf. The joined patch adds private binary32 integer-significand/exponent truncation after the exact unchanged float advance expression and finite classification by exponent. Original in-domain bit value and Q0.40 arithmetic are unchanged. Nonnegative values below 2^64 match the old valid cast, and subunit and out-of-domain values become zero. The independent UBSan oracle covers 2,525,824 binary32 bit patterns versus C cast/domain and finite classification versus the host library. Final PI32 runs have no unresolved symbols, and the disposable results sealed twice with stable sections. Both actual guarded pairs accepted, swapped model/body, absent and malformed checks rejected by the existing host guard oracle.

Compact matrix exactly preserves uncompressed Work ordering: rename LFO HZ to LFO RATE, insert LFO MODE after it, then unchanged LFO2 PWM and following pages. IDs, columns, blanks and titles retained. Uncompressed Work, compact and compact+owner host descriptor/query tests match digest 6e88a687 over 8,800 contexts and 39,310 queries each. All pass focused LFO sync (11,778), PWM2 (2,074), and 49,152 legacy stereo frames (bytewise comparison, hash only). Final adjusted source reran full 11,778 clocks fixture after both target compatibility fixes, including 18 time divisions, binary32 neighbors and 8,001–192,000 Hz. Work packet already recorded the separate 640-record authored-clock test; no raw bank files needed or copied for this join. No native boot or replay.

## Reproduce

Take the exact sibling pwm1-lfo2-fit/page-matrix/HANDOFF.md stack through PWM2 and compact pages. Apply compact-sync-source.patch here instead of the upstream uncompressed Work source patch. For the second arm apply prior control-owner-pointer.patch afterward. Existing normal-link-only and guarded HELP companion drivers are unchanged; same flags and map reader; guard follows existing palette-safe HELP protocol. The host-only Work fixture applies separately, as do the test oracle and pre-existing descriptor oracle. No production map edits, emulator, device images or releases.
