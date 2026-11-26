# SuperJ6 Sub: absolute held-level attribution
Current original receiver has identical returned left and independently sampled post-IIR left samples across both scored holds. Original measured before/after IIR via .3b78e0/.3b7934. Original media in memory only; instrument graph held LFO/effects are not represented by isolated physical projection. Installed dry captures are only separate terminal comparison. Compact insertion is telemetry only: actual DSP and PCM bit-identical to existing Range-corrected signed bench. Compact pre-IIR with VCA is a chronological same-stage comparison manufactured only for measurement; stock applies actual velocity/AMP after the filter. No normalization, gain fit, limiter ablation or emitted sample assets. Removed-reducer figures are arithmetic attribution, not measured alternate audio.

Replay from lane scratch root (restore Range-corrected donor via sj6-cross-preset/sub-range.py if necessary; tap.py requires its exact hash):
```sh
set -e
python3 sj6-sub-absolute/tap.py /tmp/sj6-cross-range.c /tmp/sj6-sub-absolute-core.c sj6-sub-control/compact.c /tmp/sj6-sub-absolute-driver.c /tmp/sj6-sub-absolute.f32
SJ6_RENDER_SOURCE=/tmp/sj6-sub-absolute-driver.c SJ6_DNBX_CORE_OVERRIDE=/tmp/sj6-sub-absolute-core.c bash sj6-cross-preset/build.sh sj6-hp-exact-exp-fit/kept-elite-1 /tmp/sj6-sub-absolute
/tmp/sj6-sub-absolute sj6-cross-preset/fanin-range.private/sub-only.private.txt /tmp/sj6-sub-absolute-final.f32
cmp /tmp/sj6-sub-absolute-final.f32 sj6-cross-preset/fanin-range.private/sub-only.private.f32
OPENBLAS_NUM_THREADS=1 PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench python3 sj6-sub-absolute/probe.py /tmp/sj6-sub-absolute.f32 /tmp/sj6-sub-absolute-final.f32
```
