# Five owner-designated BREAK assets

These are the five selected encoded zones from the preserved original
`slots-before-install.x0xslots`: BR1 zones 1–3, followed by BR2 zones 1–2.
All five are the requested default selection.

The .adpcm files preserve the complete original encoded zone bytes. The .wav
files are deterministic mono 22,050 Hz, 16-bit PCM previews decoded from those
bytes. They exactly match the source contract's preview and PCM hashes. They
are not original pre-backup WAV files; those filenames and lossless provenance
are unknown. No normalization, resampling, pitch or tempo change was performed.

Current owner direction authorizes public Git and Library distribution of
these owner-selected assets. No CC, GPL, public-domain or other specific audio
licence is asserted. Preserve actual recovered notices and attribution.

The GPL-3.0-only notice in verify_assets.py applies to verification source code,
including inherited Felucca IMA decoder semantics, not to the audio payloads.

No installer, device write, sample erasure, package replacement, deployment,
hardware acceptance or complete historical sample restoration occurred.

## Listen and choose which of the five to preload

Preview WAVs (listen before a fresh install) live on the nightly site, not in this
firmware tree:

- Page: https://gobbajogger.github.io/DnBOX-nightly/starting-breaks/
- WAV: https://gobbajogger.github.io/DnBOX-nightly/private-audio/<id>.wav
  (also without the .wav suffix). <id> is personal-br1-zone1, personal-br1-zone2,
  personal-br1-zone3, personal-br2-zone1, or personal-br2-zone2.
- Nightly tests: tests/product-starting-break-preview.test.mjs and
  tests/product-break-http.test.mjs in gobbajogger/DnBOX-nightly.

On a fresh install, choose all or any subset with the host installer (no new UI):

    tools/fm1_install.py PACKAGE.fwsc --fresh-install
    tools/fm1_install.py PACKAGE.fwsc --fresh-install --default-breaks all
    tools/fm1_install.py PACKAGE.fwsc --fresh-install --default-breaks br1-1,br2-2
    tools/fm1_install.py PACKAGE.fwsc --fresh-install --default-breaks none

A later install, or a zone or slot the user cleared, is never refilled.
