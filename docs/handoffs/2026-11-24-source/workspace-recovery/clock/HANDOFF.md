# Synth One native owner-clock port

Scope: original RATE/BPM (ID64) and Factor on the existing b7–9 typed carrier.
No device writes, preset changes, or run-time tests. Preserves original typed
integer BPM 1–200, all 19 original factor stops, transport arbitration, arp Gate
eligibility, and wait/reset behavior. External fractional Ableton Link is not an
SMK panel field. Host baseline remains independently selectable.

1. On the restored guarded carrier source: `python3 recovery/clock/clock_native_apply.py TREE --check`.
2. Apply with the same command without `--check`.
3. In addition to existing typed-native flags, add
   `-DX0X_S1_FENCED_CLOCK=1 -DX0X_S1_CLOCK_NATIVE=1` consistently across TUs.
4. Link both boards, then later compare long-run source timing and performance
   against the authenticated original. No exact-tree fit or timing claim yet.

The helper replaces compiler double math and per-sample modulus with an explicit
binary64 nearest-even addition and a cached strict threshold. Positive modulo
wrap agrees with threshold crossing in the original panel domain, where one
phase increment is smaller than every factor. Rare visible step evaluation
separately emulates rounding the original binary64 quotient before truncating.
It is not an ordinary fixed-point approximation.

Genuine JieLi pi32v2 leaf-only compile under -Os/-Wall/-Werror succeeded, with
no undefined symbols. Text sections: delta 116 B, add 276 B, step 224 B,
boundary 120 B. These totals are not a whole-firmware size: inlining, factor
table, scheduling integration and linker deduplication are unaccounted for.

Both read-only anchor validation and apply succeeded on genuine archived
full-controls postimage hunks with exact recovered runtime/trial headers.
The isolated compile of the resulting two actual clock bodies also passed:
1,214 bytes text and 76 bytes factor table, no undefined symbols. pi32v2
static assertions confirmed equal size and alignment of clock fields vs the
old host-double field layout. This ABI scaffold does not replace an exact
firmware link. The five-file overlay depends on the missing exact source
recovery; rerun `--check` against the restored carrier before applying.
