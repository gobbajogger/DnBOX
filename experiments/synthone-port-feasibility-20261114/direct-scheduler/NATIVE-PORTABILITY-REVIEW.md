# Independent source/native-object portability review

**ACCEPTED within the bounded source/native-object scope. No retained findings.**

Accepted source-manifest SHA-256: `d138334eb8a6eb800b1022a8d58b828aafe149d2408d1e3977184bdd46c8d58c`.

Accepted two-package freeze SHA-256: `0e7835aa3ce1bba4fa3a4edc08fedc079fc0ed165b1ea3fc8c3afe1aee25b151`.

Baseline: #940 `b0013c8a8d7209a155494f5e9b4952dedde87ba2`, tree `ce48b94f3e667fcefb0a0d0d8adeb8390816f372`.

The initial P2 is closed. The native runner now supplies an explicit minimal compiler environment instead of inheriting driver/loader/search controls. The saved regression fails before the repair and passes afterward. A fresh independent recheck confirms that `CCC_OVERRIDE_OPTIONS=+-ffinite-math-only`, together with other poisoned parent settings, no longer changes any of the 17 runtime objects' executable sections. Three compiler/header/control substitutions still reject before build creation. Both independent native builds have verified object/dependency identities and unchanged compiler-evaluated layout.

The five-file repair changes only the runner, its regression and documentation/receipts. All compiled runtime, oracle and supplier inputs remain unchanged. Initial independent O2/Os scheduler fixtures, full supplier comparisons, metadata/hardening and FixedArray probes remain applicable. The sanitizer fixture passed with leak detection disabled; default LeakSanitizer explicitly fails under ptrace and is not a pass.

Target-ABI layout remains: 4-byte pointers; Scheduler 9,956 bytes, common State 9,552, Ports 396; seven note banks 3,108 and 53 table descriptors 1,484 bytes. These are object/type measurements, not fit.

This acceptance excludes native DSP execution, executable/firmware link, table-read placement/workload, Juno composition, linked resource/free-pool/stack/CPU/device fit, whole-engine/global-FX and release qualification. The initial REQUEST CHANGES receipt is retained as review history.
