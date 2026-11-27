# 8,768 B/owner Synth One delay cap — rejected

Uses the already-authenticated original-source graph on both arms; only the delay is replaced. The candidate is the existing seeded, random-access 128-sample predictor with dynamic 4/5/6-bit selection and unchanged causal write order. No preset-specific modes, EQ, gain or time alignment. The three-history variant already gives the codec an advantage over a stricter four-history exact implementation. Scope is host DSP comparison, not linked firmware or an entire compact engine qualification.

Cap is deliberately optimistic: 6,784 B pre-existing scoped lease plus 1,984 B per owner from 3,968 B measured safe paired spare POOL, although this specific combined ownership is not established. The one-line diff is `27752u-HEAD -> 8768u-HEAD`; candidate uses 6,732–6,804 B payload + 1,960 B state allowance, 8,692–8,764 B total. The original graph's pre-existing unused histories remain fixture allocations only; nothing here represents firmware footprint. See `cap/compare.json` for all current source-signed comparisons and exact output SHA hashes.

Exact-cue observations (best adaptive arm):

| Fixture | Whole signed residual | Stereo spectral distance |
|---|---:|---:|
| Irresistible Forces Pad | −16.29 dBr | 11.20 / 10.70 dB |
| BB Kubrick Hall Drone | −13.15 dBr | 10.66 / 11.73 dB |
| Deeper Space Pad | −20.74 dBr | 10.62 / 12.33 dB |
| BB Stunned By Splendor Drone | −25.52 dBr | 4.54 / 6.61 dB |
| Mushroom | −26.72 dBr | 1.67 / 1.77 dB |
| Dark Theater Organ (delay bypass) | Bit identical | 0 / 0 dB |

No rejected or clamped tick in the static authored cues. Sanitized warm test (**352,800 samples, 44.1 kHz**) follows real four-line original delay with 2.4 s retime, independent sources, resident history, .982 feedback, bypass/reopen, and return to .036 s. The bounded cold-policy accepts 92,008 updates and rejects 260,792. Pre-reserving 2.4 s accepts all but leaves only 1.00–1.01 kHz stored sample cadence, and the fractional variant gets only −0.118 to +0.047 dBr by second. No ASan/UBSan findings, no admission. Receipts `cap/{cold0,reserved0,reserved48}.{receipt,warm}`.

The target physical tradeoff cannot be fixed within this implementation by better key choice: warm-retime chronology needs all future-readable old intervals; cold-scoped memory loses legitimate time control, while a full-support reservation strips usable bandwidth. Hall already failed at 27.7 KiB/owner, and existing measured F32 candidate needs ~40 kHz to satisfy the isolated spectral threshold. At even an optimistic three histories of total duration `5T`, a 2.4 s hold at 40 kHz needs 480,000 cells: the cap supplies only 0.113 payload bit/cell (6,808 B before headers). This is a structural constraint for independently represented history, not a general lower bound for every possible architecture. No full bank rerun because holdouts already falsify the candidate.

## Reproduce

From `s1-native-fit-frontier`, with pre-existing authenticated S1 source closure:
```sh
mkdir -p cap
sed 's/27752u-HEAD/8768u-HEAD/' ../s1-portable-delay/ring.c > cap/ring.c
sed "s@str(D/'ring.c')@str(P/'ring.c')@" ../s1-portable-delay/build.py > cap-build.py
python cap-build.py cap
OPENBLAS_NUM_THREADS=1 S1_SHAPES=0,1024 python ../s1-portable-delay/compare.py cap cap/out 62 113 154 191 151 366 >cap/compare.json
```
For warm stress use the existing `s1-portable-delay/warm.sh` compiler command, replace only its ring.c path with cap/ring.c, and do the three settings in the signed receipts. `cap/out` and executables are scratch, not a source bundle.
