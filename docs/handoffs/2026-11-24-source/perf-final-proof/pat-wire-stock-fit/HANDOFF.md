# Exact PAT-wire tip: stock Elite/Pro link only

All 314 postimage entries verified before and after the isolated build against ledger f7d17a572dc1affdde651cb8764857da92ddb6308a3194d2bb72618ecbf98c12 at composed-pat-wire-current/POST.sha256 (the pointer resolves to the stage directory). Fresh objects, link script and map on both models. All 14 pinned generated headers match the preceding f77 stock links byte for byte. Only exact current source, no subsequent ID56, alternate region or experimental audio codec.

| Stock map | Elite | Pro |
|---|---:|---:|
| App demand | 582,100 B | 582,088 B |
| Physical XIP capacity | 552,672 B | 552,672 B |
| Physical overflow (real ld error) | 29,428 B | 29,416 B |
| App increase versus f77 run | 812 B | 808 B |
| POOL usable after reserve | 540 B | 540 B |
| POOL margin above hard minimum 512 | 28 B | 28 B |
| Unassigned ordinary static RAM tail | 1,016 B | 1,016 B |

Both attempts fail link due to physical XIP; neither has undefined references or a successful ELF. Mapped POOL 0x51de4 active of 0x54000, minus unmodified 8192 B reserve. .bss endpoint 0x01c1fc08, ordinary RAM upper bound 0x01c20000. .xpool/.xdata and RAM/core1 text sizes unchanged from f77. Static measurements do not qualify boot, runtime stacks or timing.

Reproduce from a private copy of source, deleting preexisting *.o, *.elf, *.map and generated app.ld in the copy, preserving input-generated headers. From isolated tree invoke the existing native-link-only.py helper, with pinned JIELI_TOOLCHAIN and AC79_SDK, PYTHONPATH pointing to copy, X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 X0X_SYNTHONE_PATCH=1 X0X_S1C=1; --board smk37 --model elite/pro. Wrapper does not build/package the loader or firmware. No emulator or device work.
