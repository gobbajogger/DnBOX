# Compact physical-range dictionary

This separate research variant preserves the recovered optimization without
rewriting the accepted compact profile or its frozen review snapshots. The
23 physical float fields still use their exact binary32 endpoints; ten unique
range pairs plus 23 byte selectors replace 23 repeated pairs. State layout,
interface and synthesis arithmetic are unchanged.

From the repository root, run the host-only comparison with a fresh output:

```sh
python3 experiments/sound-hardware-v1/hardware_candidate/size_variants/bounds_dictionary/verify_bounds.py --output /tmp/dnbox-bounds-new
```

The helper pins all six reference/candidate source inputs before creating the
output or compiling. It refuses an existing output directory. The actual
portable run passed 944 floating-field boundary checks, four oscillator-mode
checks, and bitwise audio plus complete-state comparisons after 576 calls /
31,488 frames across two owners and four rates. An incorrect pulse-width range
was detected. `HOST-RESULT.json.gz` records the final run under optimized Python; native execution was NOT_RUN. Runtime checks and C calls use explicit
requirements, so Python optimization cannot remove them.
`HOST-BEFORE-GUARDS.json.gz` preserves the earlier portable run.

`RECOVERED-RESULT.json.gz` and `RECOVERED-RUNNER.py` retain the predecessor's exact
experiment, including its absolute paths and target-object commands. That
separate prior measurement was 4,938 versus 4,859 text/rodata bytes (79 bytes
smaller). It is an object measurement, not a linked-image saving. Do not add
79 to the shared-math image saving or claim native timing/whole-instrument fit.
The optional `--toolchain` mode executes target object checks and requires its
own allocation; it was not used for the portable host verification.

The complete per-case JSON receipts are stored losslessly as gzip to keep the
review diff readable; decompress before inspection. `EVIDENCE-HASHES.json` pins
both compressed and original byte identities. No row or failed result is removed.
