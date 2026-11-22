# Firmware Local gate supplemental checks

Candidate commit `ba56645b2142b134be3aeee2236406e1c95b2bde`, tree `3093322cfeea194e4d9b190a989fe9ef4d01e874`. Consolidated base `1d727e2ba118f0c85b20f3ed241ec133e4882d7e`. Canonical invocation was `bash tools/dnbox-gate.sh integ` in the detached `source` checkout with `../firmware-gate.env`. Keep that run's exit and per-case results unchanged.

The strict preflight records 13 passed checks and three absent external original comparisons. Owner scope excludes the 9W9 and 8W8 DSP references and the TB-3PO comparison inside `sequencer`. The ordinary runner executes its own sequencer checks. It records each drum comparison as not run. No other original engine reference was fetched or used.

The untraced I4 receipt carries on its ten identical committed and working source blobs. The earlier Debian 6.12.117 guest had tracer PID zero, a clean positive process, a genuine 123-byte leak rejection and a data corruption rejection. The candidate wrapper still records the actual local `compact-programs` failure in its traced-runtime context.

## Supplemental observations

- The standalone browser safety unit and the sandboxed Playwright browser run pass with a dedicated browser scratch directory outside the checkout and no executable override. The latter opened desktop, phone portrait and phone landscape views with zero requested device calls.
- Build composition, control recompilation, synthetic validation guards and legacy 303 project adoption pass with the unchanged fixtures' nominal caller environment. The canonical wrapper inherits the stock build profile or the isolated absent-reference paths. Keep its environment-sensitive failures in the canonical ledger.
- The exact base passes both the unchanged SMC PANIC and pan-format registration. The candidate fails those unchanged assertions. Temporary diagnostic copies and source context follow below.
- In the SMC PANIC diagnostic, the third of four `start_note()` calls is the only failure. After second `setup()`, simulated `now_ms` rewinds to 1000 while prior `audio_due_ms` stays at 1085.532880. The nominal startup advances to 1080 and drains no scheduled boot audio. Candidate Bass A holds after the third manual half. With only the scratch fixture clock reset before the 80 ms startup run, all 36 unchanged assertions pass. The tracked fixture and firmware source remain untouched.
- All 22 drum pan bindings and their current runtime formatting checks run before the candidate's stale aggregate descriptor assertion. Overall check counts 197 base versus 211 candidate. The 909 KIT is unchanged at two entries. The 808 KIT expands from 2 to 16 entries, accounting for every new descriptor.
- The default environment reproduces stale host extractors for `engine-808-scratch-alias`, `engine-brk-mute` and `smk-clip-chord-display`. Compilation stops at `apply_master_base`, `d8_verb` and `cap_top`/`cap_bottom`, respectively.
- Required `lua5.1` is absent. The available local libraries belong to Lua 5.3/5.4 and cannot stand in for it.
- FWUI #992 remains a separate run at `4df0102f742417ad27e7f3510c2d89d0b430b7f4`, with 333 passed and 32 failed registrations and no saved terminal verdict. `fwui-992-preexisting.json` contains its own log hash and list of failures.
