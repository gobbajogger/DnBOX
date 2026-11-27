# Selectable compact module build profiles

Apply build-profile.patch to a scratch firmware tree. Running the existing
builder with DNBOX_ENGINE_PROFILE=synth-one uses the compact Synth One
and leaves out the *new* SuperJ6 factory backend while preserving the older
stock Juno; with superj6 it enables SuperJ6 but does not select compact
Synth One (legacy dry baseline remains). dual selects both. Unprofiled
builds are unchanged; all three retain ARP hooks. Conflicting explicit
legacy flags are refused, not silently overwritten. No dynamic code,
preset deletion or substitution rule is introduced here.

The native profile guard is separate from feature-specific controls flags.
For bank-free S1-only the existing Init-only admission must be decoupled
from SuperJ6 by the native owner. This profile does not broaden Remote
typed admission or change on-device saved-project handling. In particular,
the app must still expose disabled typed-bank state as unavailable,
instead of falling back to legacy with the same slot number; that is a
separate scoped runtime check, not certified by these profile tests.

Tools: python tools/engine_profile.py synth-one (resolve); add --elf,
--link-map and target --nm to audit unstripped final paired native
artifacts from the same build. The proof looks for both linked renderer
symbols and build objects, and refuses known compact module residues in
a disabled engine. Not a comprehensive reverse-engineering audit for
all symbols; no guessed omission from passing a configure step.

verify.sh confirms existing genuine dual Elite/Pro evidence and excludes
both single-module labels on those same binaries, plus every profile
selection, explicit contradictions, and untouched stock defaults.
Single-module linked artifacts are independently owned; this lane
avoided another native build or device write. The abandoned earlier
module-identity.patch is unrelated and not required.
