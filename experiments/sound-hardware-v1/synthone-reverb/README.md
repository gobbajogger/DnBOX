# Synth One stereo reverb on stock Elite and Pro

Two compact eight-line implementations failed the existing 1 dB level and 3 dB spectral limits on the original factory programs “Hunted pad” and “Lost in Space.” The original RevSC at 44,100 Hz takes 99,124 bytes per owner before its filters and dynamics. This packet records comparison-only code and measurements.

## Original controls and memory

Both comparison frontends applied all 144 setters in order for all 694 original programs. Of these, 543 turn reverb on with positive mix, 143 turn it off, and eight enable it at zero mix. Thirty-four use full mix. Among the 543 wet programs, 75 modulate the reverb crossfade with one or both original LFOs. Nine distinct five-control tuples drive the input compressor and ten drive the wet compressor. The resolved cutoff spans 80–900 Hz and feedback spans 0.0825000033–1. [Measured control and size data](stock-capacity.json)

In the original stereo graph, high-pass filters take the post-delay left and right channels. Each channel receives twice the filtered sample, then its own compressor. RevSC feeds another independent compressor per output. The original mix and LFO route control the subsequent dry/wet crossfade before master volume, master compression and width. All these stages remained active in the tests. The input of every reverb call matched, word for word, between the reference and both candidates.

| Original storage at 44,100 Hz | Bytes per owner |
| --- | ---: |
| Eight binary32 ring buffers | 98,760 |
| RevSC state, including eight line descriptors | 364 |
| Two high-pass filters | 104 |
| Four compressor instances with their Faust states | 384 |
| Two crossfades | 8 |
| Total before graph handles, allocator overhead or control state | **99,620** |

The independent filtered ping-pong originally reserves 7,056,016 bytes per owner for four ten-second binary32 buffers. Restored programs can demand a five-second tap. The existing authored delay witness covers reads after a live time edit, the six comparisons for the 180 Hz storage proposal, and the limits of lossless bounds when a candidate can regenerate its source. [Delay witness](../synthone-delay/README.md) · [Bounded delay comparison](../synthone-delay/bounded-delay.md)

## Two smaller eight-line networks

Both candidates keep the original eight-line scattering, random modulation, cubic interpolation and damping. Each replaces the ring words with a signed 8-bit µ-law representation and uses 946 history bytes, plus 364 bytes of RevSC state. The first advances the network at 1,470 Hz with averaged inputs and linear interpolation between outputs. The second runs the same shortened network at 44,100 Hz with feedback raised to the 30th root to compensate decay time. These prototypes measure audio and raw storage. Their log/exp companders and original full-graph host wrappers have no measured native callback cost.

Each original program restored all 144 controls, received notes 54, 61 and 66, then released its last note at 1.5 seconds in a three-second capture. Both designs matched the dry program's final output bit for bit. The two fully wet examples below failed on left and right after the actual factory reverb-input, wet and master dynamics. “June Oh 06 Sqr” passed both final channels with a light reverb mix while its isolated wet outputs failed. [Individual readings, control values and capture hashes](musical-result.json)

| Program | Candidate | Final L level / spectrum | Final R level / spectrum |
| --- | --- | ---: | ---: |
| Hunted pad | 1,470 Hz | −4.62 / 13.94 dB | −2.19 / 13.55 dB |
| Hunted pad | Shortened | +3.42 / 4.31 dB | +2.55 / 4.29 dB |
| Lost in Space | 1,470 Hz | −8.01 / 5.53 dB | −10.53 / 7.59 dB |
| Lost in Space | Shortened | +19.76 / 11.61 dB | +18.32 / 11.57 dB |

## Factory storage and selection

The existing packed reader accepted 694 distinct original bank and stored-position pairs, supplied all 99,936 physical command words in original order, and preserved the caller's output on invalid selections. At PI32v2 `-Oz` its unlinked object has 102,974 bytes of read-only sections and 190 bytes of reader text. Compressing those data sections as one XZ stream yielded 56,544 bytes before a decoder, index and workspace. The ordinary Elite application at `2996e387` has 4,812 XIP bytes free. The firmware currently stores the compact factory selection in one byte and publishes Synth One INIT. All 694 selectable records need a wider, versioned identity across projects and note locks, plus a validated cold apply through the original ordered setters for the selected owner. The factory reference proof binds the stable key to the stored position even when JSON order or UIDs disagree. [Factory admission and prior full-graph comparison](../factory-admission/README.md)

## Reproduce

From the repository root, use fresh paths for `inputs`, `build` and `out` and set `JIELI_TOOLCHAIN` to the PI32v2 toolchain root. `targets.py` and `replay.py` use the compiler commands and original source closure from the verified factory harness. A generated header intercepts only the original RevSC call, and `prepare.py` checks each change it applies to the original mono-word storage implementation.

```sh
python3 tools/restore-synthone-test-inputs.py --bundle assets/test-inputs/synthone --output "$inputs"
python3 tests/host/run_synthone_factory.py --source-build "$inputs" --output "$build" --build-only
here=experiments/sound-hardware-v1/synthone-reverb
OPENBLAS_NUM_THREADS=1 python3 "$here/replay.py" . "$build" "$out"
python3 "$here/targets.py" . "$build" "$out"
python3 experiments/sound-hardware-v1/factory-admission/packed/check.py "$build/commands"
cmp "$here/musical-result.json" "$out/result.json"
fx=tests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe
sp=firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe
tc="$JIELI_TOOLCHAIN/pi32v2"
"$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -DNO_LIBSNDFILE -I"$tc/include" -I"$sp" -I"$sp/lib/dr_wav" -I"$fx/lib/faust" -I"$fx/modules" -S "$here/soundpipe-abi.c" -o "$out/soundpipe-abi.s"
```
