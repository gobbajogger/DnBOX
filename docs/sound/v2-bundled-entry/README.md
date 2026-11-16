# Bounded original bundled-bank entry

Read-only fixed-address inspection of the pinned original ARM64 image and its
original IPA. No original code was executed. The existing pinned Capstone
package is verified before import; tool/source and archive identities are bound.
No original binary, preset payload or disassembly is published here.

On the inspected uncached/no-override branch, helper 0x100195ae4 selects the
com.AudioKit.SuperJ6-Assets bundle by identifier. Its resourceURL receives a
PresetBanks path component; a FileManager enumerator yields URLs whose last
path component passes hasSuffix("superj6.bank.json"). That path reaches
Data(contentsOf:options:) then the previously inspected JSON bank/preset
decoders. Actual stubs, chained imports, selector targets, literal construction
and call instructions are asserted rather than inferred from names alone.

The original archive's framework Info.plist has the same bundle identifier.
There are 13 bank files: the requested 12 banks / 640 presets plus a separate User
Bank containing one init preset. The earlier bundled-legacy audit explicitly
excluded User Bank; that comparison remains valid. This new receipt initially
asserted 12 total archive resources and failed; the corrected distinction follows
the actual archive, not a changed S5 target.

99 fixed static/input assertions pass. These checks establish only the bounded
entry evidence. Cache/override origins, complete selection-to-control routing,
alternate loaders, migration and actual original execution remain unproven.
Do not conclude that every legacy key is ignored, invent aliases, or mark S5 PASS.

Independent review replayed all 99 checks and accepted the bounded dataflow only.
It did not execute the original or establish selected-preset/migration behavior.
