# SuperJ6 Delay receiver diagnostic

`./run.sh` compares the guarded receiver with a direct invocation of the
historical compact delay kernel. `./run-boundary.sh` exercises the real
runtime publisher, prepare and retire hooks. This does not test the original
app against the keyboard.

The RAM-only receiver uses the existing Delay Enable, Time, Feedback and Mix
values and FX Sync bit. Its caller must already hold the audio-source mask and
must bind a disjoint, 4 KiB owner-local history arena and nonreused token. The
runtime consumes the pending record at its prepare boundary and renders Delay
after chorus and before master. No project storage or transport is added.
The current FX Sync scope is Delay only; this is not full application FX Sync
parity.

`X0X_SJ6_NATIVE_DELAY=1` is admitted only on an unversioned SMK diagnostic
with the SuperJ6 factory build enabled. The native build retains the receiver
for sizing. The shipping configuration and installed projects cannot invoke it.
