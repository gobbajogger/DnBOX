# AMP before filter: matched authored control and note boundaries

Original independently observed write sites: generated Main prefilter s0
at 0x1003b78e0 / .3b7944; per-voice post-filter at .3b7934/.3b7d3c.
Old verified fundamental source order uses oscillator × Float32(AMP × velocity)
before its independent filter. Real typed compact currently executes its
existing filter first, then applies AMP before the mixed layer sum. This
isolates only those two operations; keeps the current filter, voice ownership,
gains, schedule, FX order, seeds, exact accepted input state and note release.

Baseline and candidate use the original's 9 exact signed capture hashes,
three factory preset IDs and armed authored/dry scope. Mode applies only
typed SuperJ6 oscillators; no shipping opt-in; no source assets. Compact
Sub/Noise still share layer stage when enabled: envelope placement is not
independent oscillator reconstruction or matched native whole graph.

Measured:
* Whole native level change up to .0032 dB across the six cases.
* Held level change up to .0039 dB; all other large level deficits remain.
* Raw dry early tail +.3963 dB toward native and log-spectral RMS error
  drops .1341 dB; authored +.2375 dB and spectral error drops .1360 dB.
* Brainwave attack RMS decreases ~.06 dB, with near-zero signed change
  against native. Solidify steady source nearly exactly insensitive; dry
  and authored tails move +.0482 dB, with mixed signed change.
* Signed residual against original barely changes. No adoption follows from
  tiny tail improvement. This is a real tail-ordering issue, not held gain.

Use amp-ingress.py with verified preimage in build.receipt to make isolated
dnbx_core.c copy. Pass its absolute path as SJ6_DNBX_CORE_OVERRIDE to
build.sh on the prior donor. Then use original cross-preset export and score.
amp-ingress.json is full signed metrics; manifests show candidate and
reference SHA-256 (no private original/preset identity data beyond prior UID
and digest, no actual control values or PCM).
