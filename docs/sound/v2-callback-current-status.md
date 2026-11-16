# Callback timing: still FAIL

## Latest measured runtime: 3e41

The distinct-coordinate runtime at `3e41e4b37c91a8f30542f3cd0724c6a518d588fd` remains deadline FAIL. Its same-input normal-mode d9ae/3e41 ABBA diagnostic recorded A 11/8, B 28/28, B 11/9, A 2/2 wall/CPU misses. The separate fixed-CPU-affinity diagnostic recorded A 28/26, B 155/145, B 246/245, A 83/83. Every run retains all 6,000 callbacks; neither affinity nor a causal improvement is established. See `v2-quad-distinct/callback-abba.json` and `v2-quad-distinct/callback-affinity-diagnostic.json`.

The full 694-program source/common-FX output corpus is finite and matches the retained 1,388 PCM hashes, independently audited. That audio-equivalence result is not a zero-xrun result or an independent original-FX/app oracle. The earlier full-corpus deadline run and all later red pilots remain failures; there is no timing qualification. No further blind rerun or floating-mode change is adopted.

## Historical f13 checkpoint

The measurements and then-next steps below are retained history, not current unconsumed assignments. Upload/installation denials remain stopped unless separately resolved.

At source f13feb9f, a fresh eight-program run produced 21 wall / 16 CPU misses and zero nonfinite words. The next tracer-observing bench produced 19 wall / 17 CPU misses with actual benchmark TracerPid=0 and unchanged normal floating mode. All 6,000 callbacks remain counted in each run; no initial or tail callbacks are excluded. Fixed fixture tuning and common FX remain explicitly incomplete app evidence.

The new callback runner captures and before/after verifies the reused factory and renderer orchestration files, imports captured factory bytes, and includes their hashes. It records the benchmark's own tracer observation separately from the Python runner. Nine focused regressions pass, including missing/duplicate tracer and altered tool bytes. Independent review and full694 timing remain open.

Independent review accepted the separate, unchanged source-helper ABI: 6,385 helper changes /957,750 target comparisons /4,096 frontend frames, plus12 scenes /576,000 frames, all zero differences and zero observed control/render heap. This is not native/full-app/timing qualification.

## Preserved but not newly published trace outputs

The initial three red pilot traces are published at f13feb9f. Follow-up f13-normal, instrumented profile, and tracer-bound outputs remain preserved locally with manifests. Publication of the follow-up `f13-normal/callback-trace.csv.gz` was denied twice by automated review as allegedly above700KB. Actual gzip size60,761 bytes, base64 content81,016 bytes, JSON tool request81,107 bytes. SHA256:5133e59320055ef4e4709e9b46c32b35035189391de276d93dadb49fc9619ed1. The second attempt included exact request-byte evidence. Stop this blob action pending resolution; do not change encoding or destination to route around it. No pointer or branch ref was published for these pending new binary trace objects. The earlier LFS budget and Swift-download denials remain separate and unchanged.

Next: independently review the hardened source pin, complete all694 host timing, then repeat through the available managed host if authorized and idle. Retain every failed run. No DSP/polyphony/effect simplification or production floating-mode change is adopted.
