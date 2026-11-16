# SuperJ6 delay and compressor: what the original is

Static inspection only (Swift field metadata, symbol tables, imported classes) of the original AU
binary and the AU.appex bundle, kept box-local by DnBOX Sound. No binary was executed. Only class
names and derived numbers appear here.

| Original field (KoFMInstrument) | Class | Superclass / backend | Source status |
|---|---|---|---|
| `delay` | `TempoDelay` | `AKStereoDelay` (AudioKit "sdly") | open source (AudioKit DSP) |
| `limiter` (the "compressor" knob) | `RageCompressor` | `AKCompressor` → Apple `kAudioUnitSubType_DynamicsProcessor` | **closed (Apple)** |
| `verbLimiter` | `AKPeakLimiter` | Apple PeakLimiter AU | **closed (Apple)** |
| `stereoWiden` | `Fatten` | contains `AKDelay` → `AVAudioUnitDelay` (`_OBJC_CLASS_$_AVAudioUnitDelay` imported), plus AKPanner, AKDryWetMixer | **closed (Apple)** delay |
| `reverb` | `EpicReverb` | `AKCostelloReverb` (soundpipe revsc) | open |
| `chorus` / `j60v2Chorus` | `TempoChorus` / `J60V2ChorusEffect` | `AKChorus` / own Swift | open / own |
| `crusher`, `autopan`, `finalDCBlock` | `BitCrusher`, `TempoAutoPanner`, `AKDCBlock` | AKDecimator, AKAutoPanner, AKDCBlock | open |

`AKDynaRageCompressor` (open) is linked but is not the class the `limiter` field holds.

## Delay maximum time and memory at 44.1 kHz

- AudioKit 4.11.2 `AKStereoDelayDSP::init` allocates `timeUpperBound = 2.0 s` per channel regardless
  of the preset: 2 × 88,200 float32 = **705,600 B**. (The exact AudioKit version linked in the binary
  is not verified.)
- Bank usage (640 presets): delay on in 469; max normalized delayTime 0.9555. The knob → seconds law
  inside `TempoDelay` is in stripped Swift and is **not resolved** statically. Scenarios: linear 0–2 s →
  1.911 s → 674 KB float stereo; tempo-synced quarter at the slowest bank tempo (50 BPM) → 1.2 s → 423 KB.
- Either way above the 344,064 B pool; PSRAM is not approved.

## Consequence

- Delay: bit-exact is possible in principle (open DSP), once the TempoDelay time law is recovered; memory does not fit.
- Compressor, peak limiter, widen delay: Apple closed units. A bit-exact port is not possible from source;
  this is a decision for the owner.
