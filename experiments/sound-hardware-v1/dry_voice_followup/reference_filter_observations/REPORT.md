# Original standalone-filter observations — 2026-11-19

**Completed the authorized bounded host observation: one wrapper compile, one host link, eight serial captures, 294,912 original filter calls and 1,048,576 retained output bytes. All attempts succeeded, with no retries and zero nonfinite prewarm or output samples.**

These are **new original-component captures**, not recovered historical PCM, a full-voice comparison or a pass for an independent engine. Only the exact retained original filter object was linked. No whole-synth reconstruction, active-modulation voice capture, native target link, emulator, timing benchmark, device or RDC work was performed. The four production native plans remain unallocated.

## Numeric observations

Every record uses 44,100 Hz and a requested cutoff of exactly 2,000 Hz (`44fa0000`). Resonance is either 0 (`00000000`) or binary32 0.8999999761581421 (`3f666666`). Each fresh original instance processes 4,096 silent prewarm samples, then retains all 32,768 input-driven output samples.

The sine input is 2,000 Hz with initial phase zero. Amplitudes are the explicitly recorded binary32 values. The table reports the finite-window RMS output/input ratio over samples 16,384 through 32,767 inclusive. It is a measured window ratio; it is not an analytic transfer-function claim. Startup remains in each full retained stream and in whole-record statistics.

| Input amplitude, binary32 | Resonance | Steady-window RMS ratio, dB | Whole-output RMS | Full-record peak |
| ---: | ---: | ---: | ---: | ---: |
| 9.99999974738e-05 | 0 | -12.582742084530 | 1.66130733001e-05 | 3.83736551157e-05 |
| 9.99999974738e-05 | 0.899999976158 | 8.219748693180 | 0.000181949524252 | 0.0002748760744 |
| 0.00999999977648 | 0 | -12.582742024813 | 0.00166130734998 | 0.00383736588992 |
| 0.00999999977648 | 0.899999976158 | 8.219749816910 | 0.0181949546833 | 0.0274876058102 |
| 0.25 | 0 | -12.582742165536 | 0.0415326841379 | 0.095934137702 |
| 0.25 | 0.899999976158 | 8.219748336554 | 0.454873808596 | 0.687189996243 |

Across these three levels only, the measured ratio spans approximately 0.000000141 dB at resonance 0 and 0.000001480 dB at resonance 0.9. This narrowly bounded observation does not establish level independence at other frequencies or amplitudes.

The impulse input is binary32 0.00009999999747378752 (`38d1b717`) at sample zero and exactly zero afterward.

| Resonance | Output peak magnitude | Peak sample index | Whole-output RMS | Last 4,096 output samples RMS |
| ---: | ---: | ---: | ---: | ---: |
| 0 | 6.19163529336e-06 | 10 | 1.14800357833e-07 | 4.20389539297e-45 |
| 0.899999976158 | 4.7774669838e-06 | 7 | 1.67011524769e-07 | 4.20389539297e-45 |

The small nonzero impulse-tail subnormals are retained in the raw files. Nothing was coerced to zero. No gain adjustment, normalization, phase alignment, resampling or tail edit was applied. The four excitation files were created once and reused byte-for-byte for their two resonance records.

All input/output paths, full hashes, window definitions, means, peaks, RMS values and frame receipts are in [observations.json](observations.json). RMS uses the square root of the binary64 accurately summed squared samples divided by the sample count; means use accurately summed binary64 samples divided by the count.

## Exact output identities

Each output is a 131,072-byte mono little-endian binary32 file.

| Case | Output SHA-256 |
| --- | --- |
| `impulse_a38d1b717_r00000000` | `a359504d52a530642a8d765d0eb63d8e07d8aa6bcb57c4c07ba2b7abadb0d113` |
| `impulse_a38d1b717_r3f666666` | `03648c81026761549fbb96d8c8caff415e2e04cc2c991423a510a7c2ed0a4311` |
| `sine_2000_a38d1b717_r00000000` | `ae61cb21601383930eb057dd43de5b27933369ba68227ff8c3750da777e15b0f` |
| `sine_2000_a38d1b717_r3f666666` | `86fe9790b66b3417aded7cbfa0bbcbdea59851736b42f1d2c2067db742a14f72` |
| `sine_2000_a3c23d70a_r00000000` | `41037bf8f0a8490c760fdd5cbcb28ecea5da6d917cf10a66b314d6687cc1e81c` |
| `sine_2000_a3c23d70a_r3f666666` | `4f2aaf3d896cf3a6ddddd98783cb7a0bbbd815170d053aa08e15b5f7ae8785fe` |
| `sine_2000_a3e800000_r00000000` | `ccecdfbfab6d86c15cf0d706b63b44e1ab512c1e70d7534c08752a60aa61b1a3` |
| `sine_2000_a3e800000_r3f666666` | `c2ca3c127f65632d618789cf1703e751560bd61308b71b725750a50b223121bb` |

## Reproducible input and build identities

- Frozen wrapper SHA-256: `2d07331b8ed8c930fa0eabce399a227bf3a6792ac970be94eee837b7533847a7`.
- Frozen plan SHA-256: `ce3c56cdfe695715586aaba640b6bb5eea8e919d6771055d154128402594a30e`.
- Exact historical original object SHA-256: `494e6c80f27c9fd81dbe9b26836ba922b239918b0fb1b570fe6041255a5f02f6`.
- New host executable: 16,880 bytes, SHA-256 `9a9d65d856922e09ef4ae2a99f4f4337251c61ce02839b2f20d2b36213839973`.
- Compiler: `x86_64-linux-gnu-gcc-14 (Debian 14.2.0-19) 14.2.0`.
- Linker: `GNU ld (GNU Binutils for Debian) 2.44`.
- Wrapper controls public API, file I/O, frame boundaries and counts only; it contains no filter algorithm.
- Both exact public-header hashes and the original object hash were checked before compilation and again before capture. The wrapper, plan and all four excitation hashes were checked before use.
- Floating-point build controls include `-ffp-contract=off` and `-fno-fast-math`. The wrapper requires IEEE754 binary32, little-endian byte order and round-to-nearest mode.

Compiler, linker, compiler subprogram, C/math runtime and loader binary hashes are preserved in `metadata/toolchain_preflight.json`, `metadata/toolchain_commands.json` and `metadata/link_audit.json`. Current compiler/linker versions were captured in retained logs. The historical runtime-library hashes were unknown; this is a newly pinned host observation.

Link-map inspection found exactly two workspace object inputs: the new public wrapper and the exact historical original filter object. Symbol inspection confirmed the four required original public API definitions and no HC, source-scheduler, compact-reader or runtime candidate symbols. The executable has only C and math runtime dependencies; loader resolution matched all pinned runtime hashes.

## Execution bounds and retained evidence

Current local capacity was checked before build and before capture. No competing compiler/linker/renderer/emulator process was found. Compilation and linking each had one 30-second bound; each of the eight serial capture processes had its own 15-second bound. These are workload limits, not a timing or real-time qualification.

Every attempted command retains stdout, stderr and its receipt, including empty logs. There were no failed or discarded attempts. The exact wrapper source, generator, four inputs, eight raw outputs, map, symbol/runtime audits and execution receipts remain under this scoped directory. Original engine source was not copied. The embedded original host code remains isolated in `build/`; it is not part of the portable numeric packet.

`numeric_packet.tar.gz` includes only the numeric streams, observations, public wrapper/generator, frozen command and hash metadata, reports and logs. It excludes the original object and the linked executable. It is suitable for an independent implementer to consume as comparison evidence without receiving original algorithms. The exact included files are listed in `NUMERIC_PACKET_MANIFEST.json`.

## Gates and remaining work

No independent candidate was rendered in this task, so no candidate error or pass is claimed. The existing dry-voice gates remain fixed gain 1, RMS error ≤ 1 dB, spectral error ≤ 3 dB and zero nonfinite samples. All prior dry/modulation failures and source-aware historical results remain separate.

The earlier 18-file / 54-note active-modulation deficit is unchanged. This standalone component packet does not establish full-voice dynamic behavior, factory-program coverage, native memory fit or real-time behavior.

**The authorized standalone observation scope is complete. No execution blocker remains for these eight records.** The next independent-engine comparison can consume the same four frozen input files and compare against the eight retained original outputs. No further original-host run is included in this completion.
