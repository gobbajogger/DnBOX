# Synth One bounded random-access source morph tables

Input: authenticated 52 x 4,097 Float32 source at SHA256 0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5, 852,176 bytes. The existing source lookup interpolates among 4,096 periodic lattice points; sample 4,096 is not separately addressed. All 13 source bands, four morph knots, wrap behaviour and 24-bit phase advancement remain. No per-preset storage, settings, or fit.

- Unfiltered baseband laws use no tables; integer samples are source bit-identical.
- Filtered triangle uses exact-at-Q14 quarter symmetry; filtered saw uses half odd symmetry. The source square equals (40/39) times the difference of shared saw at p and p−2048. The source pulse equals 0.75 + (3.2/pi) times the difference at p and p−512. Checked all authenticated source points: each relation is within 1.35e−7 before coding.
- Stored points are Q14 with fixed-shift independently indexed blocks. Each block can use an affine integer predictor and bitpacked residuals, or direct int16 fallback; some rows use short Q14 cubic/linear knots where an exhaustive per-row bound allows. The decoder directly addresses a block and its needed symbols; it does not decode sequentially, allocate history, cache wave states, or require RAM other than normal call temporaries. Mixed shapes reconstruct both original adjacent lattice points before applying original phase interpolation.
- Shared tight uses 18,472 B payload+directory; 13,224 B lower-cost data arm is supported but not run against all 906. All table bytes kept in read-only static storage; an actual native linker region is not yet selected/verified. The size is table format only, not product delta.

## Sound evidence

Actual compiled decoder exhaustively checked all 212,992 original points. Shared-tight max absolute lattice error is 7.65324e-5. 270,063 real oscillator ticks span both sides of band edges, all native band thresholds, interpolated settings/wrap, and no-bandlimit. Phase exact throughout; raw oscillator RMS relative delta 0.00243%. 18 representative and 12 worst previous full graph cases passed a same-cue test.

Full exact recompiled Float32 baseline reproduced old candidate PCM SHA in all 18 representative cases. The tight storage arm was then run on all 906 source-authored wet cases with identical reference PCM/notes/tuning/order. It retains 906/906 <= -40dBr; 890 at <= -60 (baseline 891), 706 at <= -80 (baseline 710), 39 exact waveform hashes (baseline 41). Worst still Arcade Lead at -47.56405dBr, approximately 0.418598% raw RMS residual. Maximum absolute whole level delta remains 0.00170179 dB. No new -40dBr outliers. This is host evidence, not native sound or resource qualification.

## CPU and further adoption

Lattice base: 880 B static PI32v2 code and 52 B band constants, in addition to 18,472 B table. Object compiled only, with lrintf unresolved; target phase-rounding implementation and worst-case CPU are not yet verified. The source arithmetic may invoke 8 stored primitives per oscillator tick, not 4 hot Float32 direct loads. On x86 unpaired filtered calls were roughly 76–128 ns vs Float32 9–21 ns for fixed cases. An optional adjacent-pair branch reuses same-block/shape metadata with bit-identical outputs on 372,736 probes and improves some fixed cases (~48–93 ns) but worsens a dynamic sweep (82→94 ns) and uses 1,670 B text. It is not the selected frozen bank arm.

## Reproduce
Exact restored workspace dependencies; no generated header or private arrays are included:
    bash s1-morph-compact/reproduce.sh shared-tight bank
    bash s1-morph-compact/reproduce.sh shared-tight holdouts
    bash s1-morph-compact/reproduce.sh shared holdouts

Main artifacts: encode.py, morph-random.c, optional morph-paired.inc, bank-summary.json, lattice JSON, and host/static CPU receipts. Use the proven shared-tight format first for subsequent native implementation, preserving the reference phase law. No APK, framework, coded table binary/header, or PCM is included.
