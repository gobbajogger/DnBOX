# Serial 909: matched real guarded A/B, no offload or scheduler change

The donor already contains the validated exact EXP/control reciprocal and 808
table dedup. Both synths+arp with no PSRAM, INIT-only diagnostic libraries, same
quarantined companion layout and actual manufacturer target linker. Every arm
uses the identical 922-entry heavy key tape at offset 0, model-correct sealed
synthetic project (S1 A/arp, Chicago B), 360 MHz strict virtual clock. Model
APP/RAM/protected region guards remain; full disposable NOR has identical bytes
before/after all runs. All have 319488 PCM frames, zero skips, no bad mailbox or
giveup. Neither source arm is adopted (strict target 10,809 us, zero late).

| Source | Elite peak us / late / settled reads | Pro peak us / late / settled reads | Guard APP free bytes Elite/Pro |
|---|---:|---:|---:|
| Matched exact table-share donor | 13168 /44 /3 | 13046 /60 /4 | 932/940 |
| Cache same Huffman input window | 12940 /36 /4 | 13133 /51 /4 | 948/956 |
| Independent fractional integral-step fast span | 12985 /43 /4 | 12928 /39 /4 | 356/364 |

All remain +564 B above the required pool reserve. A strict settled read count
difference does not invalidate scope of measurement, but does prohibit a
sample-exact full replay claim. Both modified DAC files differ from donor,
despite exact isolated arithmetic tests; latency/end-user parity not proven.

Huffman cache: keeps two source bytes from the most recently consumed code.
New code is at most six bits, so logical position advances by zero or one byte
on each invocation. Refill only when the ordinary path requires reading the
next code, using its existing extra tail byte verification; no read beyond the
original access set and no persistent cache added. Both ordinary and instrumented
host ASan+UBSan pass 56,000 serial DSP blocks with all four PCM buses and the
full kit state bit-identical (only TU-specific source pointers normalized).

Fraction span: target RAM endpoint matches both boards exactly at sampled rate
increment 1 + 47,139,328 / 2^32 for the actual active open hat and crash. Their
hit histories are not in the existing unit-rate fast path. New independent arm
admits step 1 plus arbitrary fractional increment only with valid one/two-code
lookahead and unreachable tail in its current span. It executes one decode per
output except carries, preserves interpolation's original binary32 expression
and writes the exact final lookahead/predictor/position state. Generic fallback
untouched. ASan+UBSan and ordinary host proof 56,000 original matches. State
report is loaded endpoint only, not proof of active voices at every worst block.

Next larger bottleneck is not yet a verified path. Best remaining debt is
2131 us Elite and 2119 us Pro (different branches) before zero-late criterion.
A complete unsigned16 exact IMA 89×8 delta table would need 1,424 B plus
reader, compared with best possible measured free APP 948/956 B on these
arms. A proposed fixed 48-row compact hot table costs 768 B alone and covers
4,077 /4,091 /4,091 of the first 4096 codes in the real generated hat, crash,
and ride streams, respectively, before ordinary fallback. The inventory is
measured source-data evidence, not a target link or CPU savings claim. External
read-only tables or user slot shrink are not authorized in this lane. Leveraging
that size spent for a small hot loop alone is not shown to cover 2.1 ms debt.
