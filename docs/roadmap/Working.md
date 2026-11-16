# Working

Consolidated source remains firmware `e742be7bd0d01323c899de6e156f40b85ee451db`
and nightly `9165adb92fcdaab756c4e59e3d4a8ee22437ca5a`. No v2 lane is merged
into those targets yet. The following are observed draft source/evidence pins,
not promises that later heads inherit every result.

| Owner | Work and observed evidence |
|---|---|
| sound | Native pre-FX owner7972B/alignment4 at866cec771c17dfbfddb88a7833755824bb4fb480; full native link, XIP fit, postFX and activation remain unqualified. |
| fwui | R22 geometry/screens and source-faithful labels; source review/render tests ongoing, no finished screen inventory claim. |
| integ | Native stock pool repair13e0b087839c9606bab98680cabf3a2d74f74757 independently measured margin836B on Elite/Pro. Stable per-part owner grouping under test, not Synth activation. |
| remote | Gesture/slider/protocol/visual work; normal-sandbox runtime and full R1–R7 qualification still open. |
| web | Real five-asset provider/build admission68f4b7b718a45291c86d4548b996c5095e8353a4. Combined nightly draftc93f1225b9eaf7735397e1a4540b8b646e868af6 awaiting exact-head gate/review. |
| test-and-merge | Independent source/host/native review and before/after Local evidence. No Review10/10 or full Local PASS asserted. |

Each owner retains one lane PR into the consolidated target. Integration owns
shared runtime/build interfaces and these queues. No main/settings/device writes.
