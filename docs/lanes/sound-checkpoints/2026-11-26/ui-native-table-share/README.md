# Independent exact native lookup/table deduplication

The baseline is the exact matched coldread+EXP/reciprocal guarded Elite/Pro
comparator, not an older apparent head. Changes source representation only:

- One immutable indexed table each for native SuperJ6 Alt3/room clock,
  shared FX/Chicago delay divisions, master compressor plus its UI waveform
  ratio, and IMA decoder corrections. Values are byte-identical and existing
  indices, clamps, arithmetic, output bytes and externally visible controls
  are unchanged. The SuperJ6 high-precision host path still uses its
  exact original double table/arithmetic.
- One immutable list of the same five displayed SuperJ6 LFO waveform names.
- Four linker-emitted identical tables of absolute 808 active-state addresses
  become one small field-offset table. It works for arbitrary kit pointer,
  not only the stock owner. The existing early-return loop remains; disable
  only compiler unrolling on clang to prevent target size regression. This
  can change native execution timing, so independent loaded A/B is needed.

Guarded native measurements:
| Pair | Elite APP free | Pro APP free | Usable pool |
|---|---:|---:|---:|
| exact coldread+EXP/reciprocal baseline | 548 B | 560 B | 564 B |
| independent source/table dedupe | 932 B | 940 B | 564 B |
| recovered APP | 384 B | 380 B | 0 B |

No layout, board flags/pools, test reservations, or BREAK space changed,
and no installed image/Git operation. Exact compiler/guard identities and
regions are in native.receipt. This does not claim lower populated
scene time, a zero-dropout pass, or a safe 909 transaction composition.

Run `bash ui-native-table-share/exact.sh PATH_TO_ISOLATED_TREE` after
applying table-share.patch on the matching source. All finite table domains
compare with original compiler-initialized arrays. 4,608 independent
old/new active-byte checks include both invalid-lane fallbacks. Real 808
renderer old/new output including L/R, sends, active and meter is
byte-identical across all 11 default tracks, natural releases/retriggers
and 1,200 varying-length blocks per track. IMA known oracle passes
3,193,789 comparisons each GCC optimized and ASan/UBSan
(LeakSanitizer unavailable under this sandbox). Tests compute hashes;
neither PCM nor firmware is included in source-only evidence.

A broader neutral UI outlining attempt regressed APP; not part of
this patch. The first offsets variant also unrolled the active scan and
wasted about 258 text bytes; final measured patch retains bounded loop.
