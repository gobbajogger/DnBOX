DnBOX stock firmware and five starting BREAK samples

Select x0x-smk37-elite.fwsc for SMK-37 Elite or x0x-smk37.fwsc for SMK-37 Pro.
Use the normal firmware installer. Keep a verified backup of any user samples.

The five original 22,050 Hz samples ship in DnBOX-starting-breaks.x0xslots.
Once DnBOX is installed, use Restore in Samples with this bundled file. The
archive targets slots 1 and 2 and the standard Restore command replaces those
slots. Confirm both destinations are empty and fit the archive first. If either
contains a sample set you want to keep, do not restore into that destination.
A full import needs two compatible slots; stock slot 3 is too small for either
complete group.

The ZIP transports the firmware and samples together. The .fwsc updates the
firmware, and the separate .x0xslots Restore supplies the samples. See
MANIFEST.json for model identities, CRC-32 and SHA-256 checksums. The license
files and the starting BREAK asset notice accompany this bundle.
