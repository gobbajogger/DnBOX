# Original Synth One host factory comparison

Sources derive from AudioKit/AudioKitSynthOne
`6466a3715c96b7ecf1dd255a218cbd571408a314`; original MIT notices remain in
frontend/notices and fullfx/notices. SOURCE-MANIFEST.json pins the copied
source-derived frontend, original Swift loader/default source, full FX bodies
and Soundpipe units. This is test code, not active firmware DSP.

Run from the firmware repo:

```
python tests/host/test_synthone_factory_generator.py
python tests/host/test_synthone_factory_runner.py
python tests/host/run_synthone_factory.py --source-build <verified-original-build> --output <fresh-dir>
```

`--build-only` compiles the exact closure but emits audio NOT_RUN, never PASS.
The source build must contain its original validated COMPILED-SOURCES.json and
CLOSURE-SHA256.json, the source closure, command recipe, and these byte-pinned
inputs:

- public-morphs.f32:852176B,
  SHA2560d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5.
- compact.bin:165292B,
  SHA256a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5.

The runner stages only explicitly verified input files. The generator and150
physical descriptors are also staged and hashed. Compiler dependency files
check that actual frontend/FX sources and headers belong to the bound closure;
unlisted include shadows cannot count as tested inputs. The result records the
runtime TESTED-SOURCES hash and factory closure/commands hashes. All source
objects are rebuilt, not borrowed from an old donor build.

The694 non-User presets cover all12 upstream bank files, preserving duplicate
entries/UIDs. This is NOT the separate640-preset SuperJ6 corpus. Commands follow
96 direct original Swift setter bindings plus48 sequencer writes (144total).
Unknown/non-DSP fields are preserved in source JSON, not silently aliased.

Threshold fixed before execution: zero different stereo float words and zero
NaN/Inf. Both paths use the common original FX; only pre-FX source-vs-compact is
independent. Two-second scenes do not cover every long tail or control path.
No Apple UI/runtime, tuning/hold/modwheel, callback xrun, native fit or complete
lane qualification is implied. Do not mark a missing test PASS.
