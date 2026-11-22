# Pro normal WIDE panel detents

The unmodified Integration replay at source `1c46c6fc8d7a550b3308515a3415ea63d86e7486` recorded 43 late halves and 43 xruns at a peak of 14,380 µs. Wave and Width both advanced live from 64 to 127. The Pro patch ended square/square. The wet renderer stayed on shared short alignment and the worker recorded zero give-ups. [Integration receipt](https://github.com/gobbajogger/DnBOX/blob/0afa1f1c8ee6fb45d62121f37d89bae2f44ce743/evidence/integ-resume/normal-wide-wet-1c46c6fc/README.md)

The counterfactuals below kept the same package, ELF, flash seed and public emulator, previously verified by SHA-256. Each used the strict unshifted eight-second 360 MHz software replay. The only event changes were dropping the Wave detent `4560:e4:+80`, the Width detent `4565:e6:+80`, or both.

| Scene | Peak µs | Late halves | Xruns |
|---|---:|---:|---:|
| Archived unmodified | 14,380 | 43 | 43 |
| Without Wave detent | 10,691 | 0 | 0 |
| Without Width detent | 14,351 | 53 | 53 |
| Without either detent | 10,811 | 0 | 0 |

Removing Wave eliminated the deadline misses with the Width detent still present. Square-to-square typed rendering is the next path to profile. The counterfactual rows do not satisfy the unchanged normal-WIDE gate. [Result hashes and timing](results.json)

Run with the archived assets extracted from `lane-integ-resume-0130-1c46c6fc-normal-wide-software-only.tar.gz`. Use the recovered Integration source supplying the exact strict timing runner and the official Linux x86-64 `fm1-live` 0.5.1 binary. The wrapper checks the package, ELF, flash, scene and emulator hashes before replaying.

```sh
python evidence/sound-resume/normal-wide-detents/replay.py INTEGRATION_SOURCE ARCHIVE_DIRECTORY FM1_LIVE FRESH_OUTPUT
```
