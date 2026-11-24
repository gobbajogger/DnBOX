# Synth One PAT SOUND boundary (host-only)

Exact preimage verified with every listed source entry in
perf-final-proof/s1-osc-mix-lfo/POSTIMAGE.sha256 (manifest SHA256
f77d26a5a3a51956f398d677155612a278e0f3c059dad5c1f1bb4c68c042fc09).
Use a copied folder; apply ui-only.patch, copy the test to tests/host,
and run this packet's run-host.sh from the copied source root, passing
the authenticated S1PK and an isolated output folder. Exact S1PK SHA
c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44.
Pre UI SHA 6e4e50106bc7c06a32a69c0c51699cb773cceb2be14670f5facc96d5db281e04;
post UI SHA 93b4e2c6add4ce23447876181be8f5a5a6262f2df6f2d6610d7a0b5ab440fbd9.

Unpatched UI narrowed 695 to 183, then encodes unsupported ordinal 2 as
OFF. Patch only touches ui.c: descriptor bounds real pattern wire to OFF
and INIT, label displays INIT, and early boundary return preserves dirty
state and historical C1–C4 precise bytes. Not a new program wire encoding.
Synth One part SOUND picker, wire codec, engine and persistence untouched.

Host test uses production UI input, project save/load, sequencer and
engine with fake panel/storage. Physical SELECT and K1 detents prove both
bounds, no dirty mutation at either bound, raw historical alias persistence
and first actual sequenced onset under INIT after cue/reload from a
different real factory preset. Chicago/Miami/Juno pattern counts, first
wire selections/readout, and OFF keep their existing behavior. Real part
SOUND K2 crosses 255→256 and 693→694, clamps at 694, and goes back; both
bass owners. Exact 694 S1PK plus INIT, not the separate uploaded 906.
176 host checks passed on the patched baseline. No native/build-fit,
device, Git write or release claim.
