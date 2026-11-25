# Actual PI32v2 tests of the storage limits

These are data-only experiments using the real JieLi compiler and linker.
They cannot create an update or run on hardware and do not substitute for
a complete firmware build, decoder integration, or migration.

`link_residency.py` generates the actual exact, independently removable
bank payload (1,546 presets) from the original assets and the exact packed
five stock BREAK loops. It uses real sector addresses; when a sector cannot
be placed, the source assigns it to a zero-length linker region so the real
link reports the precise missing bytes. The control allocates an oversized
private virtual test region and checks all output bytes and addresses by ELF
section. The two backup directories are charged at 4 KiB apiece; their
sample contents are lab-only, not a candidate storage format.

| Lab condition | Native demand | Potential capacity | Overflow |
|---|---:|---:|---:|
| Presets, all five BREAK loops, A/B directories across all three slots and proposed 16 KiB journal; 909 and Chicago assumed relocated | 284 KiB | 248 KiB | 36,864 B |
| Same with the observed largest update's 20 KiB staging reserve | 304 KiB | 248 KiB | 57,344 B |
| Banks only in conditional conservative empty extents (remaining audio and directories already charged) | 176 KiB | 16 KiB | 163,840 B |

`app_relocation_floor.py` separately assembles only the measured packed 909
samples (50,961 B) and exactly reproduced Chicago A/E coefficients
(6,099 + 15,629 B) and attempts to place those 72,689 B in the last proven
combined-diagnostic APP slack. It excludes the other required bytes and any
decoder. The native map shows the exact asset size and refuses the measured
352 B Elite / 360 B Pro windows: short by 72,337 B and 72,329 B respectively.
A separate full-size control links. This is a measured lower bound for
relocation into those **unchanged** APP windows, not a claim about a
future rewritten firmware.

The conservative journal C specimen is isolated in `journal-native/`.
Its -Os selector+encoder is 1,424 B text/RO with no static initialized or
zero data. Actual product incremental cost, runtime call-stack budget and
migration writer are not included.

## Run

Point the following paths at the verified local handoff, actual repository and
private user inputs. No preset contents, compiled assets or raw audio are
included in this checkpoint. Native experiments use a private temporary
directory and remove it on exit.

```sh
python link_residency.py --offline <preset-offline> --helpers <private source> \
  --superj6 <J6presets.zip> --synthone <Synth-one.zip> --firmware <fw checkout> \
  --codec ../stream_storage.py --toolchain <JieLi root>
OPENBLAS_NUM_THREADS=1 python app_relocation_floor.py --firmware <fw checkout> \
  --codec ../stream_storage.py --toolchain <JieLi root> \
  --filter-generator ../chicago-filter/filter_words.py
```
