# Authored SuperJ6 mod-wheel destination correction

Apply qualification-mod-wheel-route.patch after qualification-lfo1-seq.patch.
One shared admission predicate now decides both whether the LFO1 destination can be selected and whether the physical wheel can drive it. The legacy PWM route still works; the authored LFO1 A targets are accepted; bypass is not misclassified as active.

Complete native object build: Elite and Pro. Guarded linker fit on that exact tree: Elite +104 B XIP, Pro +116 B, both +632 B above the existing 8,192 B RAM reserve. Source pin: c13f003d78f2e50b128d6551667fef602f620ce5b7de93cf441c64c587d18a85.

Exact-source focused host test passed: all 68 original menu destination IDs admitted; the physical mod-wheel receiver audibly changed the first and sixteenth arp step routes without changing the other owner or persisted sequence. Original signal parity and full loaded performance are not established by this test. Strict populated scene at preceding LFO1 candidate: 21 Elite and 17 Pro late blocks across six offsets per board; not publishable. No install, release, or device write.
