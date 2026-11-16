# Source-exact tuning reference preparation

This prepares a standalone Swift execution of the pinned original AudioKit4.9.2
classes, not a translation of their math and not an original iOS app execution.
The only class-text edit removes `@objc` exposure annotations for Linux. Exact
original MIDI type, range clamp and negative-Boolean helper fragments are kept.
A host sample-rate setting and no-op diagnostic logger replace platform plumbing.
No A4 callback, preset hold behavior or NPO routing is invented.

`tools/run-synthone-tuning-reference.py` snapshots all authenticated original
source bytes and all694 factory inputs before generating its sole Swift compile
unit. Original bank bytes are consumed from the same captured objects that were
hash-verified; the script does not verify one file and then reread a live path.
Each factory master set is supplied to the original table class, or its original
default constructor is used for a missing set. Processes at44100,48000,96000Hz
keep the original static Nyquist initialization scoped to one sample rate.
Output records exact Double and Float bit patterns for128 notes per program.

Six focused closure tests PASS: original math unchanged except the declared
annotations; original-byte corruption rejection; path escape rejection;
ambiguous fragment rejection; changed staged Swift rejection; and exact binding
of generated694 cases to captured original bank bytes. These tests do not run
Swift or qualify audio. The local actual compiler probe returns NOT_RUN because
swiftc is absent. A separate managed-environment read-only probe is underway;
the available Mac is offline. No installation or budget-denial workaround.

```
python3 tests/host/test_synthone_tuning_reference.py
python3 tools/run-synthone-tuning-reference.py --output <fresh> --stage-only
python3 tools/run-synthone-tuning-reference.py --output <fresh-with-swift>
```

Future execution reports EXECUTED_SOURCE_MATH, never a full sound checklist
PASS. Original app callbacks, A4 retuning, note127 copy behavior, MIDI sustain,
modwheel, actual audio, Swift-platform differences and native resource admission
remain separate required evidence.

The managed probe also found no Swift compiler. A subsequent proposed isolated
local official toolchain download (about1.1GB compressed) was denied as outside
the authorized read-only/no-installation probe. No retry or indirect download
was attempted. Root handles explicit approval; Swift execution stays NOT_RUN.
