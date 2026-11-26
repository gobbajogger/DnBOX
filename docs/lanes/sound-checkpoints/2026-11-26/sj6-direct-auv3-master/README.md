# Direct SuperJ6 AUv3 master comparison

The official installed Mac build (0.999.8 build 9, `aumu/j6ct/AuPr`) ran directly in a small AppKit/AVAudioEngine capture host, not a DAW or the standalone instrument. It was loaded out of process for all four passes, with the app's main queue free to deliver the asynchronous plug-in load callback. MIDI frame positions, signed stereo 44.1 kHz PCM, the authentic Brainwave Mono Bass preset identity, all intended control edits, and PCM digests were verified. The host has no live audio output or device writes.

Main and Sub were captured separately, at authored Master (0.3150142729282379) and exact half Master. Across all frames, half Master changed output by −6.02020 dB on Main and −6.02059 dB on Sub; compared with ideal linear scaling, residuals were −42.00 dBr and −52.47 dBr respectively. Both the held chord and the release tails show the same level law. This rules out a nonlinear Master value response for these inputs; it does not establish absolute gain elsewhere in the chain, remove the compact-vs-reference mismatch, or validate the unablated effect graph.

`main.swift` implements the direct host; its bundle needs the pinned `s4-inputs.json` resource. `score.py` validates and scores the local output folders. The render archive has SHA-256 `82d0348669ebecb49245ee83fcb44c2a8576b9a2a5599e2f4591e671646bd5b8`. No original program binary is present.

## Reproduce the comparison from Git

From this directory, run `python3 score.py` with NumPy installed. The four
`native-warm-brainwave-master-*` directories contain the original stereo
float32 reference renders and their capture receipts. No plug-in installation
or Library download is needed to score these existing files. To verify output
against the checked-in report, run `python3 score.py | diff -u master-result.json -`
(some NumPy builds may round the last reported decimal place differently).

Reference renders are saved in both Git and ChatGPT Library. The separate
Library archive also preserves the full capture receipts.

## Repeatability of the native reference

`python3 repeatability.py` verifies the audio hashes and matched capture
metadata, then scores the earlier native repeat against the latest authored
capture for each path. Whole-render level changes were −0.000294 dB for Main
and +0.000003 dB for Sub. Around chord onset, the waveform residuals were
−47.47 and −73.89 dBr respectively. These are measured source-repeatability
figures for this cue, not firmware acceptance thresholds.
