# R10 input ownership and recovery correction

This successor restores existing native control behavior across LIST, ASK and modifier chords. It corrects prototype fidelity; it does not add new firmware features or complete the whole overhaul.

[Interactive prototype](workbench.html) · [exact inputs/limits](READINESS-AND-LIMITS.json) · [manifest](ARTIFACT-MANIFEST.json) · [control consequences](CONTROL-CONSEQUENCES.md).

LIST sound edits now change base values rather than hidden step locks. PATCH+K2 changes clamped tempo and consumes PATCH release. BT+REC keeps REC/transport unchanged and asks to clear the selected part; the question identifies its then-current pattern and warns that909 also owns shared FX/master motion. Isolated clear Undo/Redo is modeled, while native quiet grouping, global history capacity and active-capture endpoint finalization remain outside this fixture. Keyboard legends match the model MIDI convention without asserting measured hardware pitch.

Selective ASK priority is retained: PATCH+PLAY performs Redo and leaves the question/caller intact; plain PLAY answers NO then toggles once. ASK+REC stays masked. The prior LIST history and GEN SAVE-modifier fixes remain.

Four producer240×240 inspection renders: [clear scope](native-dispatch/clear-909-question.png), [LIST base edit](native-dispatch/held-PART-LIST-base-edit.png), [held PATCH](native-modal/question-PATCH-held.png), [Redo acknowledgement](native-modal/question-after-Redo-original-releases.png). They are proposal pixels, not current firmware captures. Actual current references remain separately versioned in private verification; no equivalent whole-state pair is fabricated.

The [mobile PDF](../r8/DnBOX-R8-mobile-comparisons.pdf) and R7/R8 stills retain their original identities. They are not new R10 visuals; actual iOS opening remains untested. This correction packet is not another owner-facing whole-design presentation.

Final-input producer checks pass within their stated scopes. Independent affected assessment is pending; no numerical count establishes acceptance. The full FX/controller, populated BREAK, real connected use, recording strategy/lifecycle, whole-project history, durable recovery and reserved component outcomes remain unfinished. No firmware edit, build, device, merge or release is authorized by this artifact.
