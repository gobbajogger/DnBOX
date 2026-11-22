# Four stock native fit executions — terminal evidence

## Result and scope

All four allocated stock jobs completed in the exact requested order. **All308 native tool commands exited0:292 serial compilations, four physical-map links and12 symbol/section/disassembly inspections.** The shared-header image saves **6784 bytes on both Elite and Pro**, with no change in the size of any non-text memory section. Every child was awaited and each owned local process group was observed released before the next plan started. The allocation in PR973 comment6995594729 is consumed.

**Target firmware code execution: NOT_RUN. Emulator execution: NOT_RUN.** These are actual compiler/linker/resource measurements, not sound, numerical identity, behavior or timing acceptance. No loader/package, device write, fullsuite, gate, production activation or RDC operation occurred. Root was explicitly notified of the four-job release before starting its next separately allocated work.

| Plan | Physical link exit | Load image incl.4-byte marker | XIP free | Margin above1024-byte reserve |
|---|---:|---:|---:|---:|
| production-reference-elite-04 | 0 | 551492 | 1180 | 156 |
| production-shared-elite-05 | 0 | 544708 | 7964 | 6940 |
| production-reference-pro-06 | 0 | 551484 | 1188 | 164 |
| production-shared-pro-07 | 0 | 544700 | 7972 | 6948 |

All four use the physical XIP start0x02000120 and capacity552672 bytes. Each image size independently recalculates from the maximum load-section end, rounded to4-byte alignment, plus the4-byte marker. Alignment adds0 bytes in these four images. The marker was counted from the physical map; no firmware package or executable image assembly step was run.

The reference images pass narrowly:156 bytes above the reserve on Elite and164 on Pro. Shared increases those margins to6940 and6948 bytes. These numbers apply to the exact frozen stock profiles below. They do not replace the earlier compact-fixture results, and no result is extrapolated between models.

## RAM, pool and map sections

Every image has pool335092/344064 bytes used, leaving8972 bytes raw free. That is780 bytes after the8192-byte reserve, or **268 bytes beyond the required512-byte margin**. Ordinary RAM extent is97204/98304 bytes, leaving1100; it includes600 bytes of `.data`,96596 bytes of `.bss` and8 bytes of intervening alignment. XPOOL uses9216/9216 bytes, leaving0. Header sharing does not recover any of this RAM.

| Section bytes | Elite reference | Elite shared | Pro reference | Pro shared |
|---|---:|---:|---:|---:|
| .text | 547572 | 540788 | 547564 | 540780 |
| .ram_sink | 1024 | 1024 | 1024 | 1024 |
| .ram_text | 2708 | 2708 | 2708 | 2708 |
| .c1_text | 608 | 608 | 608 | 608 |
| .data | 600 | 600 | 600 | 600 |
| .c1stack | 4608 | 4608 | 4608 | 4608 |
| .bss | 96596 | 96596 | 96596 | 96596 |
| .pool | 335092 | 335092 | 335092 | 335092 |
| .xpool | 9216 | 9216 | 9216 | 9216 |
| .noinit | 308 | 308 | 308 | 308 |
| .ble_ram | 3888 | 3888 | 3884 | 3884 |

The section sizes above were parsed independently from the saved physical maps and matched the historical runner's result values. Only `.text` size changes within each model pair. Other section load addresses shift with the text end as expected; this is not a RAM-layout change. Every ELF's symbol output has no named undefined symbols and no detected soft-double routines. Each saved `.ram_text` disassembly contains0 calls. The `.c1_text` section retains its existing2 call instructions; it must not be described as call-free.

The shared images retain one symbol for each of the six selected functions. Five are weak global functions and `fm_logf` is a local18-byte function after link-time optimization. This supports actual native function sharing; it does not prove arithmetic identity or runtime cost under the native compiler.

| Function | Elite reference exact-name symbols | Elite shared exact-name symbols |
|---|---:|---:|
| fm_exp2_nf | 1 | 1 |
| fm_exp2f | 2 | 1 |
| fm_expf | 2 | 1 |
| fm_log2f | 1 | 1 |
| fm_logf | 1 | 1 |
| fm_db2lin | 1 | 1 |

Detailed per-model symbol lines and sizes remain in `STOCK-FIT-SUMMARY.json` and each native plan's `symbols.log`. The shared declaration change leaves source arithmetic unchanged, but target audio/state/timing qualification remains outstanding.

## Input identity and live-head reconciliation

The allocated plans retain source commit **e0ef1994a91abef619c755f9428e44c8c9f064c4**, tree9d90b64c610126c5602a12e2409bc60305f4a02d. Root's allocation receipt separately records live PR973 as open draft at **f7f27cde98f7ac7594eb4ad23526652f0fb10130**, with consolidated/base76040daa5b52d686a8b7b544296ba5d79610a95d. The plan identity was not relabeled.

A read-only Git blob comparison found **all351 selected canonical source/metadata paths in the published f7f27cde tree byte-identical to the shared plan inputs**. Reference fastmath deliberately reverses only the six sharing declarations. Current dry research/docs changes were not imported into the plans. The comparison is recorded in `PUBLISHED-SOURCE-RECONCILIATION.json`.

The other six frozen inputs are the five generated headers plus the model physical linker script. Generated-header identity and the **default empty BREAK bank** are unchanged. No generator ran. The profile retains the official development `_900` identity and official stock app source/flags; nightly/release identity, regenerated fonts, factory BREAK content or changed compiler settings need matching measurements. These stock plans include the existing stock Juno/backend closure; they exclude the compact core and the unwired Synth One envelope unit. Production Synth One silence and complete independent instrument quality remain unresolved by these resource links.

Before each plan, the guarded verifier checked all357 frozen files, exact plan hash, toolchain binaries,107 target headers,187 Clang resource files, clean include/library environment, unoptimized Python and absent prior execution receipts. All commands entered through `python3 verify_inputs.py --execute --plan EXACT_PATH`; no direct historical-runner command was used. Saved compile and link argv match the corresponding PLAN exactly. Post-execution hashing confirms every original input file and PLAN byte remains unchanged.

## Actual execution and local release

| Plan | Outer start UTC | Outer end UTC | Elapsed s | MaxRSS KiB | Admission MemAvailable bytes |
|---|---|---|---:|---:|---:|
| production-reference-elite-04 | 2026-11-19T00:27:19.725948Z | 2026-11-19T00:27:33.827749Z | 14.101813 | 83056 | 8530751488 |
| production-shared-elite-05 | 2026-11-19T00:27:43.501373Z | 2026-11-19T00:27:57.631484Z | 14.130233 | 83152 | 8598605824 |
| production-reference-pro-06 | 2026-11-19T00:28:06.244848Z | 2026-11-19T00:28:19.273120Z | 13.028349 | 82924 | 8663662592 |
| production-shared-pro-07 | 2026-11-19T00:28:35.692024Z | 2026-11-19T00:28:48.945284Z | 13.253267 | 83172 | 8485285888 |

Elapsed values are observed build durations, not firmware timing. MaxRSS is the Linux wait4 value for the verifier process and its waited descendants; it is not a sum of resident sets or a per-command RSS series. The historical command receipts record each command's start/end and exit, while the outer receipts record this resource high-water mark.

Fresh local admission used a3200000000-byte MemAvailable floor and1000000000-byte free-disk floor. All four passed; the local cgroup memory files were not exposed and are explicitly recorded as unavailable rather than inferred. No relevant active compiler/target job was observed in this local PID namespace at each admission. Root's prior-job release supplied coordination. These observations **do not establish the absence of unrelated/global processes**. Each job's own process group had no remaining local members after wait4, and all command intervals plus all outer job intervals were sequential.

Raw `RESULT.json` files retain the historical runner's ambiguous `native_execution: NOT_RUN` field unchanged. The actual308 compiler/linker/inspection receipts prove native **build** execution; the aggregate explicitly separates this from **target code execution: NOT_RUN**. No saved failure or field was rewritten to improve the result.

## Exact ELF identities and evidence locations

| Plan | ELF SHA256 |
|---|---|
| production-reference-elite-04 | `262aecf1718137d0210f18d6a601e2df977ec8897c35dffa2cf201311f3aba6b` |
| production-shared-elite-05 | `18fd069d556c6d882e69b42e2c37e0a2deb2cc81274e19160dc5419ea22978a4` |
| production-reference-pro-06 | `1a1e8a569e5a3525fbfbf1ef776b29db764d4b8bc3f203d1831d1bc3ca38063c` |
| production-shared-pro-07 | `9738ecf42696a3c4d973c370bcc7d4eeb2ad9f9dd6f15826a82e116053ef6140` |

Each raw native directory is `/workspace/scratch/e803955aec60/fit_next_plan/native-plans/PLAN_LABEL/` and contains its unchanged PLAN, actual RESULT, COMMANDS, physical `app.map`, linked `app.elf`, compile/link logs and three inspections. This report's directory contains the verified allocation and root release, initial and per-plan admissions, outer logs/terminal receipts, source reconciliation and `STOCK-FIT-SUMMARY.json`. Root has copied the four results/admissions/terminals and raw maps-and-commands archives into its canonical evidence area and will persist the combined recovery artifacts. No extra native work is needed to package this evidence.
