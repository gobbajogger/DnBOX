# SuperJ6 isolated effects parity bench

This feeds identical deterministic stereo impulse pairs or two-tone bursts
to the selected one-effect kernel. Each arm starts from clean state, uses
44.1 kHz, and is separately measured wet/bypass. The residual is
(compact wet − compact dry) − (reference wet − reference dry), without
peak leveling or alignment. It is neither a representative preset test
nor full instrument parity.

Run portable harness checks and the reference-only Delay diagnostic:
```sh
python bench.py --self-test
python bench.py --fx delay --frames 32768
```

When the exact combined source is approved for host audio, compile its
isolated kernels and run the four-arm comparison (does not launch any
firmware build, emulator board, flash, or timing gate):
```sh
sh build-bridge.sh /path/to/pinned/qualified/source /tmp/sj6-fx.so
python bench.py --fx delay --compact-so /tmp/sj6-fx.so
python bench.py --fx j60-i --au "$SJ6_BIN" --au-bench "$BENCH/bench" \
    --compact-so /tmp/sj6-fx.so --stimulus tone
```

- **Delay:** invokes the existing Build9-qualified C behavioral reference
  from sj6-multirate, not the Python fixture math and not fresh ARM64.
  Matching 250 ms at 120 BPM (original sync bin 6, compact detent 45),
  feedback 0.99 × 64/127, max mix 0.90 wet, non-ping-pong and settled.
  Both arms continue history with exact dry=1. No claim on dynamic retiming,
  live clock, nonlinear gain stage or complete original wrapper.
- **J60 I/II/Both:** existing authorized Unicorn adapter executes the
  actual checksum-pinned Build9 isolated J60V2 ARM64 kernel. Modes 1–3,
  unity rate/depth, unity mix, ramp disabled; true stopped dry arm.
  Compact uses corresponding production J60 render kernel and unity defaults.
  Classic chorus is separate; use the preserved classic modes oracle.
- **Raw RevSC:** optional revsc-kernel mode executes original ARM64
  create/init/compute/destroy using pinned symbols and source ABI. This
  produces a raw Costello tail, not the original Epic wrapper or a
  Room I/II match. Does not admit a compact arm.
- **Room I/II and Widen:** bridge the actual compact callable only. No
  independently verified complete original Epic HPF/limiter/type/wet graph
  or AVAudioUnitDelay Widen path is wired, so the reference remains unavailable;
  the script does not report a null for these arms.

The runner has no Unicorn Python module. The raw ARM64 paths were therefore
not executed here; they fail closed on changed binary hash, entry symbols,
or unresolved ARM imports. No compact candidate was compiled or rendered
during source-only staging. The portable impulse/self-test and existing
C Delay reference-only outputs are in selftest.json and
original-delay-diagnostic.json. No AU, PCM, or presets stored here.

Integration validation: built the isolated bridge against older restored
`source-rebuild/predecessor` only and ran 32,768 samples of the 250 ms
case. Both bypass outputs were bit-exact. The compact wet output differed
from the verified behavioral reference; see
`known-compact-delay-diagnostic.json`. This did not touch the new
complete-controls candidate, emulator board, or release gates.
