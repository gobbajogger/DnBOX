# SuperJ6 direct Vintage knob

Apply the Vintage DSP and UI patches after the prior Build9 pitch bend correction, and add `-DX0X_SJ6_VINTAGE_NATIVE=1` to both model flags and the explicit wrapper whitelist. This is the verified physical app Vintage detune control, ID 123, not a hidden modulation destination. The new compact K4 route on FX MOTION stores owner-local position, reads it back, restores it at Init, and changes the amount of independent attack detune on the two layers without retuning a held note. It preserves bend layering. The newly wired source compiled fully on both native models before any new link or runtime checks. Source pin 58020661df0ce9ddc6baf8e9dba4b2f0fa070dd605653a63cc72fb14d4aed9ed.

The exact-cold-tables patch applies to the emitted diagnostic combined-link.py after expanding source flags; it moves only the named byte-identical read-only tables with hashes (no budget enlargement). Model pairing, payload/APP guards and no-physical-install limit still apply. It is not a deployment migration.

Previously linked exact Build9 pitch-bend candidate passed 266 focused SMK/USB checks after linking. Its full six-offset strict test failed on both boards (Elite max 12,367 µs, 14 total xruns; Pro max 13,038 µs, 23 total). Each xrun also matched a repeated non-silent DMA half. Do not carry those results over to Vintage or claim release or full direct-panel parity.
