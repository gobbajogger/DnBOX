# Preserved control consequences

| Start and gesture | Existing current firmware | Corrected proposal | Change to access |
|---|---|---|---|
| Held SEQ cell → PARA → SCALE → lower sound knob | LIST edits base; overlay excludes lock ownership | Same base owner; closing LIST restores original held lock context | No extra action; prototype bug repaired |
| HOME, PATCH held → K2 → PATCH release | Tempo clamp20..275; consumes PATCH release | Same; HOME retained | No extra action; no unsolicited Song entry |
| Stopped SEQ, BT held → REC → original releases | CLEAR selected part question; REC unchanged; consumes BT | Same question policy; adds pattern/owned-lane consequence | Same chord and YES/NO; extra information before destructive confirmation |
| CLEAR909 confirmed | All voices/hits/accents and same-pattern owned lanes, including sharedFX/master motion | Same in isolated modeled data; sound/length/config retained | No new access or simulated audio outcome |
| Clear after pattern progression while question open | Stored part, then-current pattern; separate native pattern/lane reads | Uses then-current target in explicit model boundary | No atomic race claim |
| ASK, PATCH held → PLAY → releases | Redo, question remains; caller/REC/transport retained | Same selective policy and owner/pattern acknowledgement | Same chord; ordinary PLAY still NO+one transport toggle |
| GEN edit → LIST → PATCH+REC/PLAY | Undo/Redo retain caller/list and consume original releases | Prior R9 fix retained | No extra action |
| GEN, BT held → lower knob → BT release | No GEN motion lane; consume gesture, no scalar edit/save | Prior R9 fix retained | No extra action |

All ordinary source controls and direct lower routes remain. The four240px inspection renders show only affected contexts. Current-source actions were corroborated in unchanged3c41 Elite/Pro emulator packages; the proposal is a software model and was not firmware execution. Native raw encoder cycles are not calibrated logical detents. Isolated model history is not proof of native quiet grouping, pool capacity, persistent recovery or active-capture endpoint behavior.

The earlier direct-GEN cell correction still saves two view presses in its admitted default mode; that is the existing meaningful proposal, not a benefit invented by these fixes. Full FX/MIX/master and connected musical outcomes are the next unreserved design work. Settings tradeoffs require its incumbent producer's disposition; no component was rewritten here.
