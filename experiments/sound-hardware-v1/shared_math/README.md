# Shared DnBOX math: actual Elite resource comparison

The six existing exponential/logarithmic helpers use the existing `FM_ONE`
linkage policy so native Clang can retain one outlined copy while still seeing
the body for inlining. Function bodies, constants and arithmetic order are
unchanged. Host and non-Clang builds retain static inline. The final firmware
header differs from the measured candidate only in its explanatory comment.

The two separately allocated native links ran serially on the local executor:

| Physical-map result | Reference | Shared math |
| --- | ---: | ---: |
| Compiled units | 74 | 74 |
| Link exit | 1 | 0 |
| Image bytes, including four-byte marker | 553,420 | 546,924 |
| Physical XIP free bytes | -748 | 5,748 |
| Margin above unchanged 1,024-byte reserve | -1,772 | 4,724 |
| Pool used / raw free | 335,092 / 8,972 | 335,092 / 8,972 |
| Margin above 8,192-byte pool reserve | 780 | 780 |
| Ordinary RAM used / free | 97,204 / 1,100 | 97,204 / 1,100 |
| XPOOL used / free | 9,216 / 0 | 9,216 / 0 |

The measured saving is 6,496 bytes. Both inputs use the same partial compact
Juno facade and silent S1 slot. These maps establish a resource comparison;
they do not establish complete S1/SJ6 sound, production-source target identity,
Pro fit, target callback timing or full-gate admission. Shared helper calls
also affect drums/master/FX, as detailed by the independent link review.

The [complete exact sources, physical maps, logs and serial terminals](https://chatgpt.com/library?file_id=libfile_fde3_f-4abf329e18488191bda4aef75d18e025_2021479872048191acbfdca1f870a539&account_id=personal)
are 5,292,068 bytes, SHA-256
`5ee9b04d5d875d2d5f0e743f33491a0700f051ef247938298cc31930ed564b61`.
The library bundle preserves the successful ELF as evidence; no firmware image
is introduced into this source package. The native result's `native_execution`
field means target execution and remains NOT_RUN; the compiler/linker commands
and actual terminal statuses are separately recorded.

`RECOVERED-NATIVE-RUNNER.py` is the exact historical runner, with its original
absolute paths retained as provenance. Do not run it against a new tree without
preparing and reviewing exact inputs, the physical script and a fresh output.
The original two-link allocation is consumed. Further native or emulator work
requires the next concrete local allocation; no shared RDC fallback exists.

The exact historical patch is `fastmath-shared-inline.patch.gz`; decompress it
before applying. Its mandatory blank context lines are preserved byte for byte.
`PATCH-HASHES.json` pins both representations.
