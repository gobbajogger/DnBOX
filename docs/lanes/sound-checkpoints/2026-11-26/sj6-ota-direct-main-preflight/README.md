# Original installed Main-only OTA capture did not run

Adapt only a fresh private copy of the known offline factory host:
program 2, same signed cue and 44.1 kHz/block schedule, public dry FX
toggles, an explicit Sub=0 readback, and two fresh detector-free
Main-only loops. No original software, shared session, settings, or
hardware was changed. The existing factory Raw stereo capture remains
what it was: a shared Main+Sub render, not this new candidate.

Compiled the modified Swift source into a new scratch ARM64 host with
unique bundle identity, no foreground window and 60-second self-exit,
and verified the local ad-hoc signature. LaunchServices rejected the
bundle at -10827 even though the signed executable was present. One
same-scope direct process attempt stopped at exit 134 before starting
the queue or producing PCM (no session/status, original compiler
output still available). No signed audio, real runtime filter state or
additional source admission was obtained. This is an execution
boundary, not evidence against the user's installed Audio Unit or a
reason to pause authenticated cached-source work.
