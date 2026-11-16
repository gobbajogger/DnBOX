# Actual clip-key field-domain text audit

Run `OUT=/absolute/scratch/path sh tests/host/ui_clip_keys_domain_test.sh` from this repository. The wrapper generates unchanged production CK/gfx/font code, instruments only text calls in the isolated generated host fixture, compares all 18 baseline body/strip rasters byte-for-byte, then sweeps every legal raw value of each of the 17 page fields. It uses actual setters/readback and asserts drawing does not mutate CK/settings.

The font observer uses opaque packed-mask pixels, not advance widths. It counts canvas clipping, conservative label/value horizontal bounding overlap, and text past its 120-pixel column. Explicit injected negative geometry must trigger all three counters before the real sweep. Fixed context is reset for each value; it is not the Cartesian product, held-note coverage, complete visual acceptance or a device test. Existing mocked USB/Bluetooth fixture sinks are retained. No new runtime code or output captures are published here.

Producer result: 153 states, 1,914 text calls, zero detected faults; all 18 baseline rasters unchanged. The earlier exploratory sequential-state run is retained locally but is not the final fixed-context evidence. Compiler warning `minsize attribute directive ignored` is host-only and retained in the run log.

Final domain-provenance.json binds the instrumented include and observer; setup-provenance.json describes pre-instrumentation inputs separately. Public setup paths are normalized, so both original and normalized receipt hashes are identified. Reviewer requested this distinction; corrected wrapper rerun passed. ASan+UBSan also completed without a diagnostic on identical executable inputs.
