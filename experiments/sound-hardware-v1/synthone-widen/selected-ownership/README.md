# Selected Synth One 50 ms widening diagnostic

This isolated patch applies to the sound lane at `03f92a7d94ad29bdb4a6bb238311ba12776401ea`. Its stereo stage runs on the right output of the real selected DnBOX Synth One INIT runtime after each successful part render. The left channel passes through. Source `tests/fixtures/synthone-factory/fullfx/source_fx.hpp` initializes a 0.05-second delay with zero feedback and mixes its output with the undelayed right signal after the compressor. The experiment exposes parameter 121 as the applied width after smoothing. Its scope is the final widening stage. The source's upstream modulation, feedback delay, reverb and compressor are separate stages; the current production INIT supplies the dry input in this qualification.

Each admitted owner borrows the first 4,095 bytes of its released Chicago insert and 2,520 bytes from its own backend union for 2,205 packed samples. Other union bytes hold the cursor and gain. The tap runs at zero width to preserve warm state. Both project restore and queued adoption clear history only on committed Synth One selection. Both runtime and compact-owner state gate access. The source archive stores four-byte samples, and the packed tap preserves the high 24 bits. The fixture compares every delayed word to an independently indexed full-width input history with the last byte masked; it also compares every mixed output. This covers arbitrary changes in the two disjoint spans and complete cold resets.

The host boots the application, rejects an invalid request, checks the queued Chicago-to-Synth-One transition, plays distinct live notes, and warms both taps for 17 blocks. A switches to fully wet while B stays dry. B then goes half wet. A retires into Chicago, Miami and Juno and returns to a cold Synth One while B keeps playing uninterrupted. Fully dry stock and candidate produced identical 372,736-byte PCM and 36,420-byte projects. The wet trace checked 33,024 A and 45,824 B frames with 25,147 and 40,948 audible wet frames. All three projects matched byte for byte. The wet trace needed two additional render blocks to finish the explicit preset transitions.

JieLi Elite / Pro diagnostic images measured 546,884 / 546,876 bytes, 368 bytes more flash per image. Both retained 97,204 bytes of data plus BSS and 8,972 raw pool bytes free, including the 8,192-byte reserve. Native output was linked and packaged; PCM qualification was on the host. These measurements use the standalone patch on the stated commit.

## Replay

Prepare two independent sound-lane source trees with the normal build dependencies and private original-synth inputs. From the pristine tree, run the stock fixture first. From the second tree, apply the patch and copy the existing packed tap header into the application include directory. Run from a shell with the repository's original-synth host build environment.

```sh
python "$DIAGNOSTIC/run_host.py" "$STOCK_TREE" "$OUT/stock" --stock
git -C "$PATCHED_TREE" apply "$DIAGNOSTIC/selected-widen.patch"
cp "$SOUND_LANE/experiments/sound-hardware-v1/synthone-widen/widen.h" "$PATCHED_TREE/firmware/src/app/widen.h"
python "$DIAGNOSTIC/run_host.py" "$PATCHED_TREE" "$OUT/patched"
cmp "$OUT/stock/pcm-0.bin" "$OUT/patched/pcm-0.bin"
cmp "$OUT/stock/proj-0.bin" "$OUT/patched/proj-0.bin"
cmp "$OUT/patched/proj-0.bin" "$OUT/patched/proj-1.bin"
(cd "$PATCHED_TREE" && MODEL=elite BOARD=smk37 sh build.sh)
(cd "$PATCHED_TREE" && MODEL=pro BOARD=smk37 sh build.sh)
```

`results.json` records measured hashes, model sizes and linker checks. Device cycle cost and the combined footprint with other factory stages remain unmeasured.
