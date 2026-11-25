# SuperJ6 Build9 instrument HPF ID28: original cutoff parameter law only

Preimage: exact controls-init-only/source-all-wheel. Eight-file patch and
PREIMAGE/POSTIMAGE SHA256 manifests. The authenticated original Build9 type
and fixup chain takes JNO2 hpfSlider / public ID28 highPassCutoff through
Conductor highPassCutoffControl → 0x1000f1348 → instrument/core highPassFilter,
a Swift AudioKit AKHighPassFilter wrapping AVAudioUnitEffect, sending
setCutoffFrequency:. The proven scaler is 0..800 Hz, exponent 2.42. This is
not the reverb low cut receiver (ID14). See original-receiver.py and receipt;
the authenticated NIB and field metadata confirm names and receptor class.
The historical first-order DnBOX compact filter is left in place:
exact cutoff command law is distinct from exact Apple filter response.

Keep the sole existing physical OUTPUT K1 and SMC alias, no new parameter
or page. Only changed explicit Juno HPF detents/absolute writes set
explicit mode in former-invalid Juno base cell[16] high bit, paired with
existing HPF edit mask. No-op writes don't opt in. The original low-seven
bend position and slot 17 are untouched; the existing panel accessor masks
to seven bits. Marked sound/note cells replay the original 800*u^2.42 in
the existing processor; at 0 DSP bypass remains neutral. Negative encoded
internal hpf caches authored Hz at the control boundary, without new
runtime layout bytes or per-block pow; BPM, other engines and FX unchanged.
Display remains attached to same public control; rendered label is 800 Hz
at maximum and OFF at zero. Large audio cost/Apple transfer/ramp/native
equivalence are not claimed.

Unmarked archived cells and legacy locks keep prior behavior, including
the source-all-wheel typed cold path that bypasses saved old HPF despite
stored nonzero detents. The prior direct/locked sound law is the historic
15+2000*u², normalized; old/new dry legacy live hash b75fde0f and cold hash
a54b28e9 independently match for both parts. We did not silently alter
that older cold behavior or migrate any pre-existing project; only real
new edits of an old project switch the law for that one owner.

Host: patched two-owner 260 checks; exact prior 62 baseline checks; exact
previous project reader 128 checks mounts four new genuine virtual storage
objects spanning both owners and Sound+lock versus lock-only, rejects before
any save writes. New current reader validates and cold-reapplies both
settings, real note sequencer onset restore/clear, and ongoing peer isolation.
Native/link/physical device not executed, no package/Git/Pages/banks touched.
Future qualification: range sweep in native engine heavy scene, note-lane
and overlapping motion gestures beyond simple lock/clear, independent live
Apple filter curve/ramp/resonance. No exact reference-audio parity inferred.

Repro: two isolated source-all-wheel copies, copy sj6-hpf-id28.c and
hpf-wire.h into tests/host of both, apply id28-law.patch only on new.
Export HPF_WIRE_PATH pointing to empty disposable directory. Run run.sh
sj6-hpf-id28.c <new-output> '-DHPF_NEW=1 -Wl,--wrap=juno6_init' from new
to export genuine synthetic test records, then the same on old with only
'-Wl,--wrap=juno6_init'. Unset HPF_WIRE_PATH and rerun old fixture binary
for fresh legacy comparison. Bash/pinned generated inputs as usual.
