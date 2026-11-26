# Guarded cold-APP capacity checkpoint

Three source-only changes on the carried guarded Boot/Home/Song/Clip laboratory experiment, canvas still 138 rows. No changes to sound, canvas allocation, ownership, installer safety, or feature configuration. Do not release this source: neither guarded lab image links. This is a resource checkpoint only.

| Native JieLi XIP overflow (bytes) | Elite | Pro |
| --- | ---: | ---: |
| Exact paired baseline | 1,400 | 1,392 |
| Rejected shared HELP words | 1,408 | 1,396 |
| Typed Undo diagnostic format sharing | 588 | 576 |
| + project import/stream validator sharing | 364 | 356 |
| + Editor codec writes | 188 | 180 |
| + Editor codec reads | 172 | 164 |
| + project Bass byte bound table | 116 | 108 |

Final recovery: 1,284 bytes on each. The negative remaining overflow means no ELF, no measured runtime or new SRAM availability. The APP saving can be ported independently; never add these bytes to another donor's margins without relinking. Source preimages are listed in source-preimages.sha256, source targets in source-final.sha256. The exact feature flag file and linker script snapshot establish the lab provenance. Only the three firmware files are changed in the combined patch. HELP prototype and unsuccessful minsize attributes are excluded.

Apply with `patch --fuzz=0 -p1 < cold-capacity.patch` from the exact donor, then validate SHA. Use `python3 verify-diagnostics.py (same workspace donor layout)` (checks scope and caption state), verify-editor.py (compiled helper checks; pass candidate editor.c). The project-host-extra.patch installs the independent old accept/reject oracle and checks over registered staging; run tests/host/run_project_stage.sh and tests/host/run_project_protocol.sh from the host test tree. The project host oracle covers scoped stock import revisions; typed formats retain existing separate revision admission. Guarded product linker launches via run.sh from a complete exact donor plus pinned SDK/toolchain. No artifact/device write took place.

Separate final install-ticket pack/unpack factoring (apply after the base
cold-capacity patch, preserving the exact editor codec) saves another 16 B
on both linked-size diagnostics. The combined guarded strip candidate is
still 100 B over Elite and 92 B over Pro. This neither links the full image
nor reclaims the shared 138-row display allocation.
