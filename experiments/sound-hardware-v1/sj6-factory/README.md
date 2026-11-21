# SuperJ6 factory selection and complete-preset comparison

The authenticated bank export contains 640 programs. The C reader selects each by its bank and program UUIDs or by bank index and authored position. Both lookups recovered all 90,549 binary32 values in the original field order, including named legacy fields, present zeroes and externally owned global fields. The 12 banks share 14 layouts. The firmware reader uses a 12-byte cursor. Programs retain their original metadata, tempo and tuning in the host selector. The two numeric IDs 218 and 219 denote the two unknown names in the reader only. Their sounding behavior is unresolved.

The PI32v2 object compiled with the stock compiler at `-Oz` has 374 bytes of code and 122,359 bytes of read-only data, including the identities. The production image has 6,156 bytes free on Elite and 6,164 on Pro before the required 1,024-byte XIP reserve. This bank representation exceeds that space. It is an audition and integration input.

The earlier compact application replay applied 474 programs and refused 166 programs containing a square wave without serialized PWM settings. The two layers accounted for 84 and 117 such square-wave records, respectively. The build-9 main preset pass resolves each missing PWM amount and switch independently to positive binary32 zero. The revised fixture stages those four effective words even when the serialized fields are absent. Its presence mask continues to distinguish an authored zero from a default. Only the master, the two waveforms, the four PWM controls and the four second-layer amplitude controls enter this fixture. The independent [master-volume check](master/README.md) covered all 640 records. Complete FX, additional musical controls and physical firmware bank selection remain open.

## Run the factory checks

From this directory run the following with the original `J6presets.zip`. Outputs use fresh directories. The table is generated locally from the private export.

```sh
python3 verify_bank.py /path/to/J6presets.zip /tmp/sj6-reader
python3 factory.py /path/to/J6presets.zip list
python3 factory.py /path/to/J6presets.zip matrix /tmp/sj6-selected
```

For the application replay use the compact waveform and typed AMP2 tree specified in the adjacent `sj6-amp2/oscillator-prenote.tar.gz`, then apply [the compact master-volume patch](master/compact-master.patch). Run its normal host build with this directory's `compact_application.c` as `SMC_UI_TEST_FILE`, a fresh `SMC_UI_OUT`, and `X0X_HOST_DEFS="-Itests/host -I/tmp/sj6-reader -Wl,--wrap=juno6_init"`.

### Missing PWM fields during preset selection

The archived inspection of AU build 9 (executable SHA-256 `a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1`) locates the main preset pass at `0x10026926c`. Its reconstructed loader applies the first matching control or that ID's default. Main-order positions 163–166 are IDs 78, 79, 210 and 211. All four default entries point to `0x10001f2e0` and record literal word `0x00000000`. The separate postpass can reapply LFO destination base controls 184, 186, 188 and 190 when the captured destinations are 78 or 79. These observations establish the main-pass inputs. Final modulation and complete stereo audio require a separate original-engine run. [Inspected order and default expressions](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/control_contract.json) · [Main-pass and postpass implementation](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/preset_executor.cpp)

Run the Linux regression against the compact waveform, AMP2 and master tree. It uses 128 synthetic records covering four waveforms, all 16 PWM presence combinations and explicit zero/nonzero values. Each selection follows a previously enabled square PWM setting on the same owner. The reference is the pinned Python reconstruction of the inspected main pass. The C fixture reads back both layers after the ordinary host audio boundary, checks base duty and verifies the peer's raw state.

```sh
git fetch --no-tags --depth=1 origin refs/tags/archive/sound/superj6-live-voice-20261116
python3 replay_pwm_defaults.py /path/to/compact-tree
```

The Linux regression completed 512 applications (two owners, each with 128 seeded transitions). It verified 5,120 authored readbacks and 512 defaulted PWM readbacks. The archived source-derived C++ reference also passed 15,574 main-pass checks and 624 PWM postpass checks.

The earlier bank scan found 25 programs with all four authored PWM fields. On the authenticated export, the revised fixture consequently expects 640 programs, 4,580 authored words and 2,460 defaulted PWM words. The export and pinned original AU were unavailable for this rerun. No fresh full-bank replay or complete original preset audio was obtained.

## S4 input and scoring

The fixed matrix contains the first program in each original export bank plus *Medieval Sky* and *PWM String Pad*. Each graph starts fresh at 44.1 kHz with the authored internal tempo. A single note, then a chord and a release tail exercise three seconds of stereo output. Both arms use the exact sample positions and unity gain specified in [the matrix](s4-inputs.json). See [the original bench](original/README.md) for the restricted Linux component replay, the released/bundled bank identity audit and the headless Audio Unit host.

Each producer exports the stereo PCM, the receipt, and the applied program state as `<UID>.f32le`, `<UID>.json`, and `<UID>.applied.json`. The scorer checks the selected input against the release audit and complete record hash. It verifies the applied UID, tuning, tempo, input settings, every authored control in the applied snapshot, and the live control words emitted by the producer. The receipt includes the actual event schedule, render start, internal clock mode, fresh-instance reset, unity gain, block size, and digests of the applied state and PCM. Each channel must be finite and contain sound. The original needs the audited AU identity, installed executable digest and qualified load route. The DnBOX capture needs a trusted source revision and the SHA-256 of the independently identified production image used for that capture. Receipt checks establish consistency with these inputs. Producer provenance still depends on running the trusted bench and firmware and collecting their outputs directly.

```sh
python3 -m unittest test_comparison
python3 compare.py /path/to/original-captures /path/to/dnbox-captures \
    --inputs /tmp/sj6-selected --candidate-image /path/to/x0x.bin \
    --candidate-commit "$FULL_SOURCE_COMMIT"
```

Each channel must pass the existing 1 dB absolute RMS-level and 3 dB log-spectral limits in 20 Hz–16 kHz. The spectral calculation uses 2,048-frame Hann windows, 512-frame hops and a reference-relative −80 dB floor. Unusable captures yield exit 2. A fully scored matrix outside tolerance yields exit 1. The recovered material supplies zero qualifying pairs. The seven Linux component replays have their own input and output hashes.
