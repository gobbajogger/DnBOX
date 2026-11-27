# SuperJ6: voice convergence and actual sampler-to-buffer wrapper

Read-only sound diagnosis, no audio or original binary in this source bundle.

From the matching recovered source workspace:
```sh
python sj6-mix-boundary-recheck/check.py
python sj6-mix-boundary-recheck/stereo.py
export PYTHONPATH="/tmp/sj6-fx-py:$PWD/ab-effects-readonly/sj6-fullbench/bench"
python sj6-mix-boundary-recheck/wrapper.py
python sj6-mix-boundary-recheck/buffered.py
SJ6_ABL_FALLBACK=1 python sj6-mix-boundary-recheck/buffered.py
```

The runner resolves its owner-supplied current private reference image under
sj6-modern-private-recovery/arm64.private. It refuses a different SHA. No
reference bytes are in this package. The only framework stand-ins are
closure/owner storage, retain/release and stack-guard scaffolding. The
Brainwave receiver control fixture is projected, not loaded AppKit state.
The trace exercises actual sampler code, event clock and buffer duties.
Two separate roles produce different nonzero PCM. Their actual
AKAudioUnitBase render-events closure and outer BufferedAudioUnit closure
(with nonzero dirty output and separately null mData actual fallback) both
give the same signed stereo words as directly invoking the current sampler.
Neither invokes Apple's external mixer. The first missing real shared sample
remains installed oscSection output, before Unison; mono/center residual
cannot establish an internal gain. Do not change knob values, mixer gain or
master based on these exclusions.

Saved conditional Main1+Main2 taps imply convergence attenuation at most
approximately 0.98 dB and no polarity flip. Actual warm native Brainwave
held/chord side residual to the comparable synthesis route is below
-139 dBr, while the roughly 12 dB discrepancy persists in mid alone.
