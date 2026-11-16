# Lossless Synth One morph data: analysis-memory prototype

This is a bounded JavaScript source/data experiment based on accepted source parent #943 (`346223e9ec6361f0d08ac72cee785a5d223d4caa`). It is not a runtime integration, a C/native decoder, a deployable asset, or firmware/audio/CPU acceptance. The current #943 Scheduler still requires raw Tables; this package does not remove that requirement.

## Public inputs and reconstruction
The authoritative source is AudioKit/AudioKitSynthOne at `6466a3715c96b7ecf1dd255a218cbd571408a314`, under its MIT license copied in notices/SynthOne-MIT.txt. input_manifest.json pins the original paths, byte sizes, SHA-256 and Git blob SHA-1 for all 52 logical JSON rows and two ordering/frequency metadata files. Duplicate blobs permit fetching 50 row files. No waveform sample arrays, owner preset banks, binary container, SDK or toolchain are included.

Read each exact public file through a trusted source reader, for example the GitHub connector with repository, ref and path from the manifest. Supply row records `{content,returned_blob}`. The manifest includes the two small metadata texts. Convert each original JSON number to binary32 LE using DataView, in original order, and append one positive-zero guard. First four source types are 1,2,0,3; remaining rows type10, all phase0. reconstructCanonical authenticates inputs, preserves word bits including negative zero, checks canonical corpus digests and creates the 52-to-50 mapping 0..47,44,45,48,49. The raw canonical digests are corroborating reconstruction pins; source authentication comes from the exact original public bytes/blob identities, not the producer's encoded digest.

## API and execution
Evaluate compact_analysis.js in ordinary JS memory; its last expression returns S1CompactAnalysis. It also exports CommonJS when available. No network, filesystem, compiler, process, credentials or external package is used by the module.

- `runSyntheticTests()`: self-contained finite-word cases exercise serialized raw/base/affine/quadratic blocks, aliases and guards, plus the supported-mode encoder regression.
- `runFullTests(sourceRows, manifest, foldConfig)`: authenticate public rows, rebuild the complete morph container, decode every logical word, test malformed metadata and read boundaries, prove a finite wrong-word comparison detects content changes, and re-encode deterministically.
- `reconstructCanonical(sourceRows,manifest)`, `buildMorphContainer(input,foldConfig)`, `admitCompact(Uint8Array)`: lower-level source reconstruction, cold encoder and cold parser/read interface.
- The cold encoder accepts dense plain Arrays for physical/logical rows, mapping, bands and fold config. Every logical uint32 word must equal its mapped physical word, including aliases and the positive-zero guard; alias row names need not match. Bands must remain finite and strictly increasing after binary32 rounding. Invalid input throws before returning a container. Every successful build is checked against its own v1 admission contract. Tests include sparse mapping, conflicting logical alias, band overflow and band rounding-collapse regressions.
- A successful admission returns `read(row,sample,out)`, accepting integers row0..51 and sample0..4096, storing one uint32 word in a plain writable `{word:...}` data object. Invalid coordinates or null/non-object output fail without modifying that plain output. Proxies/accessors, arbitrary mapped pointers and post-admission byte mutation/concurrent mutation are outside the contract. The byte buffer must remain readable, alive and immutable. This is not arbitrary-pointer safety or C owner/alias proof.
- Structural admission is not canonical-content authentication: a finite wrong word can be structurally valid. A separately pinned whole-asset digest remains necessary. Unexpected failure is failure, not successful fallback; no DSP/block rollback is promised.
- Digest helpers support the bounded inputs in this experiment (at most16MiB per byte/string input), not an unbounded hashing API.

Minimal ordinary-memory usage:
```javascript
const api = eval(moduleSource);
const synthetic = api.runSyntheticTests();
const result = api.runFullTests(originalRows, inputManifest, foldConfig);
```
Any source-file transcription must hash-match SOURCE-MANIFEST.json before evaluation. The container digest in input_manifest.json is a producer reproducibility pin, not an upstream signature or independent acceptance.

## Format v1
All integer fields are little-endian. Fixed 64-byte header: sixteen uint32 fields are magic0x314c3153,version1,physicalRows50,logicalRows52,words4097,blockSize64,descriptorOffset64,mapOffset1264,bandsOffset1316,indexOffset1368,blockCount,blockPayloadOffset,auxOffset,totalBytes,descriptorBytes24,flags0. The map is52 bytes; bands are13 binary32 words.

Each 24-byte row descriptor has six uint32 fields: mode, rotation/sign flags, baseCount, firstBlockIndex, exceptionOffset, exceptionCount. Modes0/1/2 mean full/half/quarter orbit basis. Rotation occupies low12 flag bits; mirror sign is bit12, half-period sign bit13. A mode3 full-row RLE instead uses fields3/4 for absolute run offset/run count and requires field5 zero. RLE and exceptions use uint16 end/index plus uint32 exact word. Runs end strictly increasingly at4096. Exceptions use strictly increasing original sample positions below4096.

Half/quarter folding uses exact sign-bit XOR and exact sparse words, never floating approximation. The guard at sample4096 is the verified positive-zero format invariant. An offset index gives a bounded independent block. Its two-byte header is mode/width. Supported block modes are0 raw32,1 unsigned base+packed residual,3 modular affine+zigzag residual,4 modular quadratic+zigzag residual. Stored anchor counts are0/1/2/3 uint32 respectively. Residuals are LSB-first bit packed; all reconstruction arithmetic is modulo2^32. The predictor includes triangular i*(i-1)/2 for local i<64. No decoder history or mutable cache is required by this format; JS allocation behavior is not a freestanding no-heap proof.

The selected morph-only container measures165292 bytes: header64 + descriptors1200 + map52 + bands52 + index4804 + blocks158956 + exceptions/RLE162 + padding2. Keeping the required raw sine16388 bytes and tuning512 bytes yields182192 bytes of asset/input accounting. Sine/tuning bytes are not included in the morph digest. Native decoder text, binding/state, stack, admission temporary memory and real bus/cache/cycle costs are NOT measured here. This is not combined product occupancy or storage placement.

## Remaining gates
Actual C reader and metadata-only admission, alias/lifetime/error proof in C, unchanged oscillator and scheduled PCM comparisons, sanitizer qualification, genuine retained native link and numerical policy, complete current product memory/stack/CPU and hardest configurations, safe storage placement, whole-instrument/FX fidelity, device and release gates remain open. No protected Bluetooth/calibration/user/sample/store/OTA region is allocated. No numerical policy is relaxed and no double helper is renamed or hidden. This package must not be used as permission to execute around a read-only mount.
