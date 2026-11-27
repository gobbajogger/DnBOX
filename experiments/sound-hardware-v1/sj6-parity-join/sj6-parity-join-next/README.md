# SuperJ6 matched-source composition, host-only

All six independent assignments fed this guarded host checkpoint. Production
source, presets and master behavior are unchanged. Apply the diagnostic hunks
to the exact donor below, not to a product build. Do not install or release.

## Verified

- Dry inlet: actual signed independent Main1/Main2/Sub/Noise receiver buffers
  (Main2 may be silent under these controls) from the authorized older IPA
  and newer AU. Sub uses the original 12-zone bank, retained privately.
  Unmuted Noise is a direct original-receiver positive control, not a live
  hardware input or the authored patch. Deterministic host entropy is not
  the original installed entropy. Rendered source: 30,242 frames, padded
  with 119,758 silent frames for effect tails per scenario.
- Unison: the original ARM64 pitch-shifter computes the modulo with a rounded
  1001-minus-pitch-ratio term. The old signed phase accumulation differed at
  every sample in both tested negative-shift series. The reconstructed leaf
  matched 21,000 signed original outputs at five shifts on Q4.11-aligned
  inputs. The complete compact detent sweep matched all 2,580 step words.
  This patch wires the phase law, not an exact match for arbitrary
  interpolation/history or installed panner/filter behavior.
- Classic chorus: separately checked all three default modes in the current
  genuine kernel, with rate/depth/feedback/mix explicitly assigned in
  correct original DSP order. The compact kernel (before external booster)
  hits the same lag for all 18 signed impulses, worst whole residual
  −72.93 dBr. No Classic chorus change justified by those tests. This
  does not qualify J60 or every live control combination.
- Delay: full-rate reference selected at the actual Delay seam with
  explicit test controls, preserving its memory and phase across blocks.
  No compact history format or preset importer claim. The admitted
  reference is stereo (not a separate ping-pong qualification).
- Type II Room + Rage: Room may get an independently supplied wet fixture
  at the post-HP boundary, not an inferred Apple HP response. The dry
  compressor runs before the wet return, not on their sum. Compact Rage
  transfer and pre-chorus compact HP remain proxies. Independent scoped
  Type II original RevSC reference passes with 20-bit history, −91.71 dBr
  wet and −70.51 dBr above 8 kHz. The Type I wet limiter DSP is not covered.
- Joined actual host renderer: 10 cases per original input version;
  6,000,000 signed output words, bit-exact against separate calls to the
  same selected kernels in explicit order. Identical outputs with
  two independent callback partitions, active effects together and a
  live Room close/open/close. No output alignment or fitted patch gain.
  This qualifies wiring, **not** a whole-preset comparison to the installed AU.
- Flag-off C preprocessing for both application runtime and DSP matches
  the input donor byte-for-byte.

## Reproduce and remaining work

Use the project signed-bench workspace. Build using build.sh (source input
sums in preimages.sha256), then run check.py ipa/current with Unicorn 2.1.4
and the project bench on PYTHONPATH. Prior inlet: 2a26ab0ec040 (including
af45feabc0f5). Prior Native Normal amp/filter correction: 6b64504fc22f.
The actual unmodified original apps/recordings are reference inputs, not
binary distributions in this checkpoint.

Use the saved authored/Room-off Brainwave pair and exact signed readback
harness to qualify the whole pipeline next. The current candidate must
bind the same preset/effects controls and note lifecycle before its
terminal residual can be judged against those files. Apple High-pass,
active Rage transfer and Type I wet limiter need actual model/render
evidence; no substitution of nearby unrelated kernels. The reference
Delay and Room histories exceed target RAM; Elite/Pro fit and dropout
qualification stay open. No permission or hardware dependency is
inferred from any missing render.
