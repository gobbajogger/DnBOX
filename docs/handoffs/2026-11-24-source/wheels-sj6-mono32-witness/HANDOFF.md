# SuperJ6 original Mono (ID32), physical host DSP slice

Exact independent base: verified 813 mixed active Classic modes and
24-page cache, no RANDOM2 requirement. Verified fuzz0 and 90 UBSan real
physical+audio checks there; the same scoped option also passes 90 on
893/RANDOM2 so it composes without mixing source packets. Host-only page
VOICE K1 MONO. Other positions remain empty (no Legato/Glide aliases).
No new project bytes, no persisted setting, no native or release claims.

Authorized Build9 SHA a5243e50… authentic FXController Mono/Legato/Glide
outlets correspond to Conductor +0xD60/+0xD50/+0xD68 and observer dispatch
0x10026e990/0x10026ea44/0x10026ebac → 0x1000f14f0/0x1000f1900/0x1000f1b58.
Mono tests >0, native AKSampler commands 28/29/9 are separate. The actual
voice branch 0x10036fc44–fc84, fdd4 and 0x100370058 use dedicated slot0 in
Mono; direct overlap chooses new note. Release fallback 0x10038e544 scans
positive held counts from MIDI0 upward; 0x1003701b0–1c4 passes literal
velocity100 in the ordinary nonlegato/nontarget-fixed-velocity branch.
No guess of S1 last-note priority. Original note-off outer 0x10036ff70
decrements the held count before dispatch. Source sound synth with original
Mono pitch/voice lifecycle uses existing compact AMP/filter ADSR and no
whole-app spectrum parity assertion.

Both physical owners actuate existing DSP, alternate MIDI pitch falls
back to the lowest of two held notes, background held release does not
steal voice, one active gate, and both restore Poly. Audible runtime
frames checked and peer/no project writes proven. Switching while held
(or pedal/down/tail) is refused in this scoped receiver rather than
inventing original transition behavior. Reboot/owner program resets
volatile mode; not accepted as persisted full control.

Legato worker follow-up: 0x10036fc58/0x10037016c selects nonretrigger retune
0x1003a5208 for active held fallback, distinct from 0x1003a5004. Need prove
its real amp/velocity persistence and compact detector; no label exposed.
Glide mapped original normalized 0..0.5 (exponent 1 at 0x1000f1c0c–1c); a
nonzero stored rate is not a glide enable. Native #284 gate and #1308
pitch correction/decay math still need proof. Do not copy S1 half-time.
