# Chicago UI lag native builds, DnBOX Lead report

This file is an addendum to [docs/evidence/chicago-ui-lag-20261116.md](chicago-ui-lag-20261116.md). That record is frozen at `afe743ebc006ef8a82b34b65732c698a842dda7d`. This addendum does not change it.

Reason: section 3 of the frozen record left the Elite and Pro builds **NOT RUN** on the measurement machine. Purpose: store the native build report DnBOX Lead made on its own machine, next to that frozen record. For a player, this commit does not change the firmware image. The reported sizes say tree B matches tree A for flash, RAM and pool.

DnBOX Lead ran these builds and reported the figures below. This repository's CI did not run them. The sound owner did not run them. This file records that report. It does not add a build from this checkout.

## Toolchain

The report names this toolchain:

| Input | Reported value |
| --- | --- |
| SDK | AC79NN V1.2.14, tag `5fa84a9b` |
| Compiler | JieLi clang 4.0.1 |
| PADCAL | `PADCAL=0` |

## Trees

| Tree | SHA | What the report says |
| --- | --- | --- |
| A | `94197fc839b00aff47b98ca02f3c6396e4a6eabf` | without the fix |
| B | `02121895e268e08d8809e55f9e881dc1baaf8fe6` | with fix `4b663ba2` |

`4b663ba2` is the fix commit already named in the frozen record.

## Reported builds

All 4 builds passed: Elite and Pro, on A and on B.

| Tree | Model | Reported result |
| --- | --- | --- |
| A | Elite | passed |
| A | Pro | passed |
| B | Elite | passed |
| B | Pro | passed |

B minus A is 0 B for flash, RAM and pool on both models.

`og_states` went from 98 B to 88 B. Alignment padding absorbs the difference.

Tree B sizes in the report:

| Model | App image | Flash free |
| --- | ---: | ---: |
| Elite | 544,156 B | 8,516 B |
| Pro | 544,148 B | 8,524 B |

Pool free is 8,340 B on both models. That is 148 B above the 8,192 B floor. RAM free is 2,768 B.

## Other tree

The earlier 24/36 B headroom figure belongs to #944's `e8bf6f12` combined tree. That tree needs its own combined build. These figures do not apply to it.

## Still NOT RUN

emu360 did not run. Device and hardware latency did not run. There is no hardware evidence.

This report is not a device test, an install, or a qualification pass. The frozen record's host restore counts stay in that file.
