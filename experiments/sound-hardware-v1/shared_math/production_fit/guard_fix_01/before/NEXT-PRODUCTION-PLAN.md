# Next production fit and target identity allocation

## Concrete result

Four current-production source plans are prepared and hash-verified. **Nothing was compiled, linked, emulated or timed.** The previous two compact Elite link allocations are exhausted. PR973 request6994424328 still needs an allocation before any command containing `execute` below may run.

The source is clean canonical commit `e0ef1994a91abef619c755f9428e44c8c9f064c4`, Git tree `9d90b64c610126c5602a12e2409bc60305f4a02d`. Each plan captures **73 official app compilation units and the same 73 link inputs**, with 357 frozen input files. `COMPILE-SOURCE-PATHS.txt` lists every compilation source; each `PLAN.json` records every exact argument and every input hash. `PRODUCTION-PREPARATION.json` lists the selected current paths, historical comparison, generated inputs and plan pins. `PREPARATION-CHECKS.json` records the completed read-only checks.

| Plan | PLAN.json SHA256 | Compile units |
|---|---|---:|
| production-reference-elite-04 | `54b75516b9904283b37ffd9de9a50738a5322de6bfce754ed06265e8a976f2b9` | 73 |
| production-shared-elite-05 | `a8ba4504d514db2160af53fd76db0e2441d63420a92723104d64f70ef0e7c22d` | 73 |
| production-reference-pro-06 | `a9a41f0c31110b5d8af7b17265df1cfed7b94621fc85c83d62d99ea060568650` | 73 |
| production-shared-pro-07 | `253510c1009f19be438504be7e8d0a4554f006d321f87a40c3c03d888a4b83e4` | 73 |

## What this closure actually contains

The capture calls the canonical `tools/build.py` app planner in an isolated copy, intercepts `tc_all` at its first batch and raises before execution. All subprocess entry points and `build.tc` are forbidden during capture. It uses the official stock source/flags: four assembly units; `firmware/src/x0x.c` at the official `-Oz`; the eleven app/DSP/sequencer O2 units; 51 declared Chicago/Miami and adapter units; five stock backend/runtime units; and the integer chance probe unit. These counts total73. It preserves the official per-unit LTO exclusions and O2/Os selection.

The five backend/runtime sources are `firmware/src/dsp/juno6.c`, `original_chicago_backend.c`, `original_miami_backend.c`, `firmware/src/app/original_runtime.c` and `original_sound.c`. The stock app therefore includes its existing engines and stock Juno facade. Neither the compact `dnbx_core` experiment nor the newly committed unwired `dnbx_s1_envelope.c` enters these plans. These measurements cannot accept the independent Synth One or SuperJ6 implementation, nor resolve silent production Synth One.

Each reference/shared pair differs in **only `firmware/src/dsp/fastmath.h`**. Shared is the exact committed header, SHA256 `c82737d5302d77b974d5071c9bc58915aca723a6e6f5f5185cbe43bad962189d`. Reference reverses exactly the six reviewed `FM_ONE` declarations to `static inline`; after removing comments/whitespace it matches the recovered original reference header. Every function body, arithmetic constant, guard and state boundary stays identical. Current shared executable tokens match the already measured f902 candidate; its explanatory comment does not change the compiled program. No compact-specific flags, diagnostic counter define or forced export root are present.

The fixed profile is the official default development identity `SMK-37 Elite_900` / `SMK-37 Pro_900`, with release/version/commit display macros unset. Original synth is1, Wobble DSP0, development rate-pad UI1 and chance probe1. Optional X0X diagnostic/debug/bass flags and external board/model settings are cleared before capture. The exact flags are in the manifests. A release/nightly identity, build options, fonts or factory BREAK payload that differs from this profile requires separate matching preparation; those bytes can change fit.

## Recovered-input comparison and stale-source controls

The recovered `native-baseline-76040` contains357 source/script files under the reviewed suffix set. Of these,350 exist in current canonical source: **349 remain byte-identical; only fastmath changed**. The other seven are generated artifacts absent from canonical: five `build/gen` headers and two old model linker scripts. No recovered C/header source replaces a newer canonical source. The frozen input copies use those350 current files plus the canonical `experiments/original_synth/EXTERNAL-INPUTS.json`, the five recovered generated headers and one new physical model linker script. The official ten active external-header checks complete during capture, before the intercepted compiler batch.

The five generated headers are `chicago_compact_private.h`, `felucca_font.h`, `x0x_break_bank.h`, `x0x_drum_samples.h` and `x0x_drum_tables.h`. Their exact hashes are in `PRODUCTION-PREPARATION.json`. All eight available recovered909 sample/font asset files also match current canonical bytes; their individual hashes are in `PREPARATION-CHECKS.json`. The BREAK header represents the default empty bank. It is not a factory sample pack qualification.

No generator ran. This is a deliberately frozen generated-input comparison. Regenerating headers can involve Pillow/FreeType/font configuration, host compiler identity and optional BREAK assets; `gen_cache.fingerprint()` itself invokes a host compiler version probe. The ordinary official full build/gate must later validate its own exact generated inputs. A cache key also changes when fastmath changes even if the generated samples do not depend on its executable linkage; never infer freshness from a cached directory name alone.

The unchanged historical replay preparer is unsuitable for the new canonical tree: it requires the old fastmath hash, expects generated model `app.ld` files to exist already, and does not synchronize package board state when assigning `build.BOARD`. This preparation adapter sets `fm1pkg_make.set_board('smk37')`, copies its APP_SLOT/APP_CODE_MAX into the builder, captures current official commands and derives the script from current `firmware/app.ld`. The resulting script SHA is `4a45da2deaa8825d1e20c26a92cc5f317f28151d6fc55af28ccbf3e8ee3cefdd`, exactly the physical script already reviewed: XIP starts0x02000120 and has552672 bytes. Nothing enlarges the map.

## Exact future command scope

The next fit-only request can allocate the following **four jobs serially on the separate local executor**: stock reference Elite, stock shared Elite, stock reference Pro, stock shared Pro. A successful job performs73 serial app compilations, one physical link, then symbols/sections/disassembly inspection. Across four successful jobs that is292 compilation commands, four links and12 inspection commands. A failed physical link remains terminal evidence for that plan; there is no large-map retry, no loader, package, emulator, device operation or timing task in this command set.

`native_fit_replay.py` here is an unchanged hash-verified copy of the already reviewed runner (`f896e0fd8ce2719876271476db261002d0909392e8059158427d80118a3b396e`). It is used only for `execute`; do not invoke its old `prepare` on these inputs. `verify_inputs.py` additionally verifies all357 input files, the four native tool binaries, all107 target include headers plus 187 Clang builtin resource files, absence of previous terminal receipts and unpinned include/library environment variables. The original replay checks fewer input suffixes, so retain this additional preflight.

**Only after the exact allocation is granted and root records fresh separate-local capacity/process admission**, run each verify/execute pair below, waiting for terminal receipts before moving to the next pair:

```sh
python3 /workspace/scratch/e803955aec60/fit_next_plan/verify_inputs.py --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-reference-elite-04/PLAN.json
python3 /workspace/scratch/e803955aec60/fit_next_plan/native_fit_replay.py execute --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-reference-elite-04/PLAN.json
```

```sh
python3 /workspace/scratch/e803955aec60/fit_next_plan/verify_inputs.py --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-shared-elite-05/PLAN.json
python3 /workspace/scratch/e803955aec60/fit_next_plan/native_fit_replay.py execute --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-shared-elite-05/PLAN.json
```

```sh
python3 /workspace/scratch/e803955aec60/fit_next_plan/verify_inputs.py --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-reference-pro-06/PLAN.json
python3 /workspace/scratch/e803955aec60/fit_next_plan/native_fit_replay.py execute --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-reference-pro-06/PLAN.json
```

```sh
python3 /workspace/scratch/e803955aec60/fit_next_plan/verify_inputs.py --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-shared-pro-07/PLAN.json
python3 /workspace/scratch/e803955aec60/fit_next_plan/native_fit_replay.py execute --plan /workspace/scratch/e803955aec60/fit_next_plan/native-plans/production-shared-pro-07/PLAN.json
```

The current status of every execute command is **NOT_RUN / ALLOCATION_REQUIRED**. Do not infer an allocation from these prepared commands. The four lightweight `verify_inputs.py` checks already passed; source regeneration or a move to another source head must be reviewed, not silently substituted. Inspect failed/successful raw commands, physical map, final image including marker, undefined symbols, soft-double symbols, `.ram_text` calls, ordinary RAM, pool and XPOOL. A replay return code alone does not establish every resource condition. Required XIP reserve remains1024 bytes; pool floor8192 plus512 margin remains distinct from physical RAM capacity.

The existing compact result of6496 bytes saved and unchanged pool is evidence for that earlier image only. The stock Elite/Pro load sizes, margins and quality remain unmeasured. Actual runtime capacity checks belong to root at admission; previous small native maxRSS is not a current capacity guarantee.

## Target audio identity: exact blocker and reusable interface

The canonical `tests/host/audio_path_identity.py` runner covers17 host scenarios including909/808/BREAK, mixing, FX and master behavior. It builds with X0X_HOST, for which FM_ONE stays static inline. It cannot qualify the native sharing change.

A local diagnostic emulator binary is present and its bytes were independently hashed, without execution:

- Path: `/workspace/scratch/cc2e3d785d40/private-runs/final-runner-preflight/execution-inputs/fm1-live`
- SHA256: `04c795b1961298cbd10d4bd9881ab97fde5eef7ff179f49a6b953dab78d43dc8`
- Size: 4978480 bytes.
- Local emulator source base: `6b60618c8bf344d65a166905fb746f46ab708fcb`, with uncommitted observer additions; use the pinned binary identity rather than claiming the dirty source equals the base.

The existing `dnbox/dropout/runner.py` pins firmware94197fc8 ELF/packages. Its `--smoke` observes nine seconds, with additional bootstrap/settling and isolated BREAK witness work. Its identity comparison is **observer disabled versus enabled on the same old firmware**, not this production reference/shared pair. The representative scene explicitly excludes808. Changing its supplied image arguments alone is insufficient because its pin validation rejects the new image. This runner and its existing results must not be relabeled as new optimization proof.

Its known, source-inspected short execution interface is:

```text
fm1-live --fwsc EXACT_PACKAGE --mhz 360 --shot EXACT_DURATION_MS --flash DISPOSABLE_FRESH_FLASH --usb
FM1_STRICT=1
FM1_BENCH_KEYS=@PINNED_KEYS
FM1_USB_SEND=@PINNED_USB_MIDI
FM1_DNBOX_PCM=NEW_RAW_PCM_OUTPUT
FM1_DUMPRAM=NEW_RAM_OUTPUT
# Optional, as a matched pair only:
FM1_DNBOX_MAP=IMAGE_SPECIFIC_SYMBOL_MAP
FM1_DNBOX_TRACE=NEW_TRACE_OUTPUT
```

This is an interface template, **not a qualified command or permission to execute**. Every paired run needs the exact model-specific firmware/package identity, a freshly derived symbol map for each linked image, identical non-code flash/settings/sample input, pinned stimulus and deterministic start/window semantics. `--flash` restores the whole flash over the package-loaded image and is written back at exit; blindly reusing another firmware's flash can replace the code under test. The reference/candidate initial flashes must preserve each image's app bytes and agree outside the intended code region. No FM1_NO_BLOCKS shortcut or shared-host fallback is admissible.

A complete short identity driver for this optimization still needs representative and isolation coverage that proves909 and808 activity, real accepted/nonzero BREAK contribution, FX routing and master processing, plus exact aligned PCM comparison and normalized relevant DSP state/event comparison. Raw whole-RAM equality across differently linked images is invalid because code pointers and layout can change. Whole-output nonzero alone cannot prove BREAK is present. The old diagnostic observer validates only pinned layout/symbol contracts and must be regenerated from the actual model images before reuse. No new scene, package or exact native audio identity pair has been run or accepted here.

**Smallest concrete next allocation:** these four pinned stock link jobs. A later short target identity allocation must first supply or prepare the exact successful image pair/package construction and a reviewed driver covering the affected audio paths. It is separate from full timing and from `scripts/gate.sh sound --post`; neither is newly authorized or qualified by this preparation.
