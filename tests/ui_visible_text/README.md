# Visible text on extended native frames

`final_trace.py` takes the layered F2 producer's prepared source and the primary
inventory's source-matched combined report. It derives the expected extended and
persisted-edit identities from that report and the frozen legacy inventory; no
workspace path, production SHA or frame count is built into the CLI.

The support module `run.py` and `visible-trace.inc` must remain beside the CLI.
Only Python's standard library and the existing repository host build are used.

```sh
python3 tests/ui_visible_text/final_trace.py \
  --prepared-source /evidence/extra/source \
  --expected-report /evidence/primary/combined-report.json \
  --output /evidence/text
```

This compiles the extras-only capture and the unchanged persisted-edit fixture
in one independent source copy, sequentially. `F2_EXTRAS_ONLY=1` skips the ordinary
walker. Each binary also runs two negative variants without recompilation. It
does not modify firmware source, rerun the ordinary suite or pack image archives.

The observer maintains its own committed-paint ledger across frame resets.
Actual `geo_blit` and host `lcd_fill` regions retire replaced labels, so cached
header/footer words are retained on pixel-only grid updates. Scratch font-mask
paints run with tracing disabled and never enter the ledger. Every source label
is checked with its actual native opaque glyph mask against the final framebuffer.
Fully surviving labels and partial source-string remnants are separate records;
partial strings are not asserted to be fully readable words. Alpha-fringe,
contrast, native hardware tails and external R22 acceptance remain separate.
Every positive PNG must also match the primary producer's native PNG SHA256.

The per-case contracts require the fixed pickup, safe-mode, retry, two post-crash
core variants, fault register values/positions, calibration prompts/control names,
pad-calibration orientation and persisted-edit lane labels. They reject the
previous explanatory captions in the affected cases. Missing mutants flip actual
final framebuffer glyph pixels; stale mutants change the actual native text draw
input before rasterization. Both must finish their renderer invocation and fail
the exact-caption oracle for every declared mutant target. Pad-calibration
orientation has positive-only exact-caption coverage: no mutation of that state
is claimed.

Outputs include the full JSON inventory and TSV, deterministic gzip copies,
`case-assertions.json`, `mutant-receipts.json`, instruction-review candidates,
source receipt, both positive logs and four runtime-negative logs. Source-only
boot/calibration display slices retain their primary inventory evidence kinds.
Boot safety messages are never edited by this observer.

A stronger validator can evaluate existing raw logs without another render:

```sh
python3 tests/ui_visible_text/final_trace.py \
  --prepared-source /evidence/extra/source \
  --expected-report /evidence/primary/combined-report.json \
  --output /evidence/text --parse-only
```

The receipt retains the original trace-generator SHA; the inventory additionally
records the validator SHA. A parser-only contract addition is therefore distinct
from the native observer that produced the pixels and trace.
