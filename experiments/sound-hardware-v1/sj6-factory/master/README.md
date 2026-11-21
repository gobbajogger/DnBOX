# SuperJ6 master volume

The recovered original AU applies global parameter 0, `neonvolumeControl`, after the instrument effects. The engine-only probe recorded the argument to the original output object's `setVolume:` method. The probe supplied object lifetime and libm services and stopped at the mixer boundary.

All 640 factory records contained a master volume. The original setter was measured at 453 distinct factory inputs and three additional values. The compact implementation produced the same binary32 gain for every measured input. Eight authored inputs exceed one. The largest, 1.3200000524520874, yielded a gain of 0.5332800149917603. A separate single-precision test checked 53,016,004 normal input words against the binary64 transfer.

The typed firmware entry publishes the original raw value and applied gain at a render boundary. Gain scales both channels after chorus, including the effect tail. The real application test covered 3,504 stereo frames, including 176 frames of chorus tail, plus zero, unity, over-unity and independent ownership. The combined bank test checked 3,418 authored words on 474 programs. The other 166 programs have a missing PWM setting required for a square wave. Master volume alone passed on all 640.

Apply [compact-master.patch](compact-master.patch) on top of the compact waveform and AMP2 tree from the parent directory. Its SHA-256 is `1d6f61be2e0bb1c2f5a4fa35b2f1156f3ca7618342a007da8a46f7077184a0f6`. The updated [combined application fixture](../compact_application.c) calls the new entry. Each model retained 8,972 bytes of free pool, but the links retaining the complete ABI left only 640 bytes of XIP on Elite and 652 on Pro. Both fall below the 1,024-byte reserve. The compact candidate remains experimental.

Run the component replay from this directory after applying the compact patch. Use the verified November 15 engine-only bench with its original AU and Unicorn libraries, and a new output directory.

```sh
python3 replay.py --compact "$COMPACT_TREE" --factory .. \
  --bench "$ORIGINAL_BENCH" --banks "$BANK_ZIP" --out /tmp/sj6-master-replay
```

The firmware patch and combined factory fixture were recovered byte-for-byte by their pre-outage digests. The companion test sources were reconstructed from recorded commands. The original probe output has SHA-256 `ab8cd42f45d6bbee001fe4920fea9143cbe710d2cf46534ff2ac6ed250b10e34`. Individual application, transfer and sanitizer tests finished before the workspace disconnected. A clean end-to-end rerun of the recovered wrapper and physical timing of this compact image remain open.
