# SuperJ6 factory selection and complete-preset comparison

The authenticated bank export contains 640 programs. The C reader selects each by its bank and program UUIDs or by bank index and authored position. Both lookups recovered all 90,549 binary32 values in the original field order, including named legacy fields, present zeroes and externally owned global fields. The 12 banks share 14 layouts. The firmware reader uses a 12-byte cursor. Programs retain their original metadata, tempo and tuning in the host selector. The two numeric IDs 218 and 219 denote the two unknown names in the reader only. Their sounding behavior is unresolved.

The PI32v2 object compiled with the stock compiler at `-Oz` has 374 bytes of code and 122,359 bytes of read-only data, including the identities. The production image has 6,156 bytes free on Elite and 6,164 on Pro before the required 1,024-byte XIP reserve. This bank representation exceeds that space. It is an audition and integration input.

The compact DnBOX application received both waveform selections, available PWM controls and the four second-layer amplitude controls for 474 records. The other 166 required missing PWM state for a square wave. Across the accepted selections, 2,944 authored words matched DSP readback after a real audio boundary, and the other part retained its exact raw parameter state. Complete FX, additional musical controls and physical firmware bank selection remain open.

## Run the factory checks

From this directory run the following with the original `J6presets.zip`. Outputs use fresh directories. The table is generated locally from the private export.

```sh
python3 verify_bank.py /path/to/J6presets.zip /tmp/sj6-reader
python3 factory.py /path/to/J6presets.zip list
python3 factory.py /path/to/J6presets.zip matrix /tmp/sj6-selected
```

For the application replay use the compact waveform and typed AMP2 tree specified in the adjacent `sj6-amp2/oscillator-prenote.tar.gz`. Run its normal host build with this directory's `compact_application.c` as `SMC_UI_TEST_FILE`, a fresh `SMC_UI_OUT`, and `X0X_HOST_DEFS="-Itests/host -I/tmp/sj6-reader -Wl,--wrap=juno6_init"`.

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
