# Scoped physical ingress proof: ARP SEQ

Base Git `73749f55d17a0643f1ddd072114172d29eecc2ae`; source UI SHA256 `556b804412e1560a6c481125ab915d1b68d7849ab167de7547d7ed27d30aa40c`. The newer archived full-controls UI preimage remains missing. No production source modifications.

`tests/host/synth-arp-panel-ingress.c` uses real application input, renderer, project and engine code; the only direct calls are test fixture owner select, preset Init provisioning and read-only assertions. There is no forced page change or direct edit/setting write. Tested both owners for both engines (four cases) on default host and compact-host build with `X0X_S1C=1 X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1`. Both run logs pass all four cases.

Actual journey: HOME, tap physical SEQ, three SELECT detents, ARP SEQ page. Turn physical third sound encoder to change Gate 100→99%; verify after actual audio blocks. Hold sixth step white key and turn first sound encoder to change its pitch 0→+1; release without toggling its on mask. Peer owner’s gate, pitch, and revisions stay unchanged. Press HOME and verify no lingering hold or overlay, and edited owner settings persist.

Both sets preserve the actual 240×240 framebuffer before/after. The post-edit Compact SuperJ6 B capture shows the existing colored ARP SEQ page label, independent STEPS / INTERVAL / GATE labels and explicit 99% Gate readout. This proves one existing role/route and a per-step pitch edit, not per-step gate, all controls parity, motion design, native timing, or latest new donor UI.
