## Three exact control projections on the previous six signed factory-pair cues

No overall or preset-specific audio gain. No device or firmware changes.
Same untouched timing, authenticated original signed input hashes, and source-pinned
compact voices/FX. Variant 0 produces all six PCM hashes byte-identical to previous
Library baseline.

* 1: opt-in already verified native #180 cutoff first-empty-press mechanism,
  one exact f32 authored input from the recorded active readback. Uses existing
  owner counters/controls; writes typed route before unquantized load, exact raw
  readback asserted. Both affected Brainwave pairs first diverge at frame
  38,589; compact-vs-base residual −45.35 authored/−45.38 dry dBr; native
  level deficts worsen by <0.008 dB. The other four pairs are bit-identical.
  The direct original UI control timing proof of this owner is separate.
* 2: exact source-proved authored AMP/FILTER ADSR physical panel transfer (Double
  taper and one Float32 boundary), original AMP1/2 link chooses raw source; exposed
  FILTER LINK is retained independently, explicit +0.2 s FILTER release. State
  and times then use the unchanged compact renderer (not exact native AHDSHR).
  Changes whole waveform level <0.008 dB on each, no parity lift.
* 4: exact supported 44.1k physical 10 Hz floor for the minimum cutoff readback,
  independent post-filter stage and no extra Crusher. Placed before compact
  Crusher/Room; this fixture does not claim original effect chain order as
  compact routes other effects differently. Scoped leaf passed previously vs
  pure Apple reference. Whole level gets at most +0.072 dB and first held
  single at most +0.19 dB; some tail signs move in either direction. Still
  28–48 dB down in the whole compact model, no acoustic parity.

The diagnostic excludes a quantization error or these isolated controls as
cause of the missing level across three factory programs. It does not claim
all normalized controls are implemented or that the real app AMP clock matches.
No fitted scalars, inferred phase, proprietary binaries, raw presets or PCM
are included in this archive.

Reproduce individual mode using build.sh, exact donor from prior A/B, and
the prior export/score scripts. Set SJ6_RENDER_SOURCE to the absolute path
of render-primitives.c and SJ6_EXTRA_FLAGS to -DSJ6_MAP_MODE=N. Add
-DX0X_SJ6_NOTE_CUTOFF_LATCH=1 only for mode 1. For mode 4 create
the runtime copy using hp-floor-overlay.py and set SJ6_RUNTIME_OVERRIDE to
its absolute path. No overlays on pinned input sources. compare_primitives.py
checks mode 0 against the prior signed projection before summarizing differences.
