# R22 firmware UI continuation

Owner: Dot firmware/program. Base: `94197fc839b00aff47b98ca02f3c6396e4a6eabf`.
Frozen inputs: R22 gallery, `f254bf2c890623d6d543ed3b874ea225117f1290`
drum page and `b10ca4de2816ed7c8ef8a8482357e863204602c1` renderer.
Lead and prior Dot compositor have released UI writes. The shared integration
ref stays unchanged during isolated implementation and independent checks.

## Intended outcome

Continue the complete R22-language UI, preserving all existing controls, pages,
glyph identities and real parameter laws. First deliver the reusable rendering
foundation and every 909/808 voice page in real firmware renders, not another
disconnected presentation. Then extend that same foundation to the retained
engine pages and genuine new-engine interfaces as they qualify. No R23 glyph
substitution, raw-float/unknown placeholder, fake battery percentage or new
selected-note panel is admitted.

## Foundation and drum-page acceptance

- Replace duplicate 1px hairlines with one shared rounded, anti-aliased stroke
  raster and one continuous, critically damped presentation-only value state.
- Keep the R22 drum layout and rounded tuning fork. Add short parameter captions,
  white values and safe value/edge spacing. Remove the selected panel's green tint.
- Derive decay from engine units and its exponential law over a common time axis;
  show a clear onset glyph. Drum Attack controls click amount, not time, so height
  follows amount; actual synth envelope rise time will follow true duration.
- Correct P1/tempo/stop/battery header with the actual four-level battery API.
  Retain meaningful unsaved/cue states without painting over the battery.
- Render both kits, all voices/pages and parameter extrema, including animation
  and band-boundary checks. Verify project/audio state is unchanged by painting.
- Run real pixel RED/GREEN, relevant existing UI/state tests, full suite where
  prerequisites are available, exact native resource checks, independent review
  and remote readback before declaring each corresponding stage accepted.

## Evidence boundaries and next dependencies

Current exact baseline host simulator builds. Its genuine Elite PADCAL=1 link
has 6,432 bytes free flash, below the 8,192-byte requirement. A successful link
does not qualify release. Missing reference trees, final combined resource,
target timing and device checks stay explicit. Local/pre-merge pixel and
anti-revert gates are required; no `.github`/Actions changes are permitted.
New engine pages require actual source-qualified parameter/curve interfaces,
not invented mappings or the historical RAW fallback.
