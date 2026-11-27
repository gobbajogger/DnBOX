# Recover APP bytes while keeping the new audio pool

The verified late placement puts only the three 996-byte independently owned UI allocations into the final boot-zeroed `.sbss.ui_relocated` input section. That preserves the actual 996-byte audio-pool gain without a buffer alias.

Both native builds gained 192 bytes of APP, leaving 616 bytes on Elite and 628 bytes on Pro. Raw pool is 9,908 bytes on both, 1,204 over the hard floor, with ordinary BSS gap 184 bytes. All 73 pre-link DSP/engine/sequencer objects matched the exact prior candidate byte for byte. Hot UAC, USB and SMC locations and compact access instructions were recovered. Companion payload and fixed guard checks passed.

The preceding strict loaded 922 timing failed. These new placement-only binaries have not been replayed. Do not publish. Exact private archive source/maps/logs/objects SHA-256 `faa19b2f948e3dbef12e15c8a91571c8c56d2d8ec59cfca196f44649786812ac`. Elite ELF `57a50ab2f8cd425cc0fbebb710fb80716bb9af30667d80e8af8a6c92b4fc7a5e`, Pro `749f8e5bf792ae267e4e89b2cd85164fb7f26c2fa0bd436b3b412251aba24fa5`. No raw inputs or executable files in source archive.
