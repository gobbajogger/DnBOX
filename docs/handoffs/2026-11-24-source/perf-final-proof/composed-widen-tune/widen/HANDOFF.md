# Synth One part OUTPUT / WIDEN

Exact staged source base: the composed Reverb HP lab-post pinned by
COMPOSED-HP-PREIMAGE.sha256 (manifest SHA-256
36b2216898c4607614b838c093d7e7213ba36eccc9cb84f3f407402af636ef86).
PREIMAGE and POSTIMAGE pin the four changed files. Apply/check/reverse
passed on that isolated exact stage. No external writes or native run.

Original source: pinned upstream Synth One S1Parameter.h lists Widen
ID 121 and Kernel/S1DSPKernel.hpp assigns a 0–1 smoothed toggle.
Kernel/S1DSPKernel+process.mm controls the crossfade between the
right output and its widen-loop output after master compression;
left is not altered. Compact already uses that same control
position in s1c.c and s1u_widen_run. This introduces only the real
typed target on part OUTPUT K1 with ON/OFF readout, not an original
overall transfer or stock-delay algorithm claim (compact uses
three right-only allpasses; original uses a constant delay).
Effect card and global MIX master stay unchanged.

The same 15-byte extension reserves no new primary invalid marker.
Legacy high=31 extension low header digit: 0=unset (old f8 HP
exact; HP required in this arm), 1=explicit OFF, 2=explicit ON,
3–7 invalid. HP’s position byte is unchanged; coexistence with all
prior panel, LFO, and MIX state is exact in the golden fixture.
Old readers reject records containing Widen on their normal
project and note-lock validation path before writes. The new
reader still accepts old HP Sound/lock records and existing
dual-MIX byte images.

Host: 16,908 focused checks demonstrate first physical detents
0→1 and 1→0 on both owners with authentic both-wet-off/on factory
selections. All four affected 4096/4096 pre-mixer right-channel
samples while left and whole peer stereo PCM were bit-identical.
Both enable targets and all delay line lengths and pool bytes
were exact pre/post. Test covers same-case save/file reload,
real note-lock capture/on/off rollback, and byte equality for
busy rejection. Previous exact reader 263 checks: reads old HP
cutoff and lock and rejects new Sound/lock with all six bytesets
unmodified. Reverb HP focused 1,696 and prior MIX 1,100 still pass.
Untouched INIT and 694 preset renders at 1,024 stereo frames each
remain byte-identical (SHA2 2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f).
LCDC files are actual 240×240 compact pages with real OFF/ON.

Replay: bash replay-host.sh /path/to/exact/composed-hp/lab-post
/path/to/authenticated/s1.pack (as a single command on one line).
Factory input and generated PCM stay ephemeral; no native fit,
dropout, XIP, original algorithm parity, or release claim.
Feedback/TIME/Auto Pan remain held for their earlier transition
and topology reasons.
