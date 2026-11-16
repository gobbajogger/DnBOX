# Source-compatible Conductor control experiment

Portable C++11 control/state code and an executable Python reference for a bounded, source-inspected SuperJ6 control path. This is an isolated experiment, not the original app, a complete instrument, or an approved firmware/UI integration.

The package implements ordered LFO/control stages, explicit callback commands, source-specific reset/retrigger behavior, a streaming main-preset pass, and a separately invoked PWM base-control postpass. It preserves the distinction between the 170-entry main control order, 218 default expressions and the separate engine component namespace. It does not turn unsupported preset fields into successful DSP operations.

## Run the fixture-free host checks

Requires Python 3 and a host GCC C++ compiler with AddressSanitizer/UndefinedBehaviorSanitizer support. From this directory, supply the externally verified SHA256 of `SOURCE-MANIFEST.json`:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 validate_host.py --source-pin EXPECTED_SHA256 --out ../conductor-host-checks
```

The output directory must not exist and must be outside this source tree. The runner authenticates every declared file and the complete file set, rejects symlinks, then builds only this package's own C++ tests. It does not download inputs or execute any original app, emulator, firmware, or target binary. The result is written to `RESULT.json` in the selected output directory. Do not calculate a new pin from untrusted changed files and call that authentication.

Host tests use strict C++11, `-fno-exceptions`, `-ffp-contract=off`, `-fno-fast-math`, warnings as errors, O2/Os, and nonrecovering sanitizers. If the default sanitizer invocation fails specifically because LeakSanitizer cannot operate under ptrace, that failure is retained separately. A subsequent successful run of the same binary with leak detection disabled is not a leak check or a replacement for the original failure.

## What is and is not here

- Production implementations are the controller, LFO/table/entropy/timing, ordered cutoff/tick/lifecycle, and preset-executor modules. Headers define the admitted caller and effect contracts.
- `prefix_reference_testonly.cpp` and `default_reference_testonly.hpp` are immutable earlier own implementations/data used only by host regression tests. Exclude them, every probe, and every test from any production/native source list.
- `control_contract.json` contains source-derived control identifiers, ordering and default constants. It is not a user preset bank or a general JSON schema. Unknown input records remain caller-owned; this code does not rewrite or serialize their original bytes.
- No recordings, preset banks, app images, original disassembly, SDK/toolchain files, private reports or compiled artifacts are included. Tests need none of those inputs.

The callbacks are required capabilities, not placeholder DSP implementations. A successful command test does not prove a real receiver, scheduler, model lock, entropy source or platform math provider is faithful. Complete preset loading/migration/tuning, host transport registration, nested note ownership, reentrant callbacks and the full engine/FX graph remain outside this package. No full-preset PCM equality, real-time safety, native numerical-policy pass, product storage/stack fit, CPU budget, heavy-configuration zero-dropout result or device qualification is claimed. The controller closure is binary32. A separate target link is still required before calling that a product numerical-policy pass.

See [CONTRACT.md](CONTRACT.md) for sequencing and failure boundaries and [PROVENANCE.md](PROVENANCE.md) for source provenance and transport scope.
