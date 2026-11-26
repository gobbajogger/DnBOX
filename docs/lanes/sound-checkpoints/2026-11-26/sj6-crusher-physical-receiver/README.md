# Same Build9 physical Crusher receiver

**Do not adopt the control/address labels in this historical witness.**
This experiment proves standalone setter and receiver behavior, not which
installed AU parameter reaches the setter. The stronger
[source-mapped installed evidence](../sj6-crusher-source-mapped/README.md)
traces public #108 to the Amount/Decimation path, public #109 to a
metadata-only path, and internal #110 (not a registered public control)
to the Mix path. The earlier direct-call lab bound the laws to the wrong
addresses. Do not expose #109 as a decimation control on this evidence.

Original SHA 6d163d79…; class_ro points to AudioKit.AKDecimator method list
0x10060f5a8, whose Objective-C selectors resolve to source instructions.
All setters share a [0,1] clamped ×100 Float64 → Float32 handoff to imported
AudioUnitSetParameter (global scope/element 0). Conductors and receiver
instructions execute in Unicorn, with only ObjC method dispatch routed to
verified IMPs and runtime ARC/exclusivity, libm pow (host equivalent), Booster
dispatch observation and AVAudio unit handle calls stubbed; no native DSP
simulation claim. 21 exact receiver ABI cases and seven exact conductor
ordered events verify #109 and #108 emit independent addresses, including
real Apple exact 6.059999942779541% for #109 unit 1, and Apple ID15 12.5%
for #108 unit .25. #109 also writes rounding ID4 zero. #107 resets ID15
and Booster without changing ID3 or detaching the node. Relevant physical
leaf captures are independent and do not bind actual loaded same-node DSP
behavior or partition behavior for every state. No native implementation
or output parity claim; target approximation requires a distinct exhaustive
range/rounding test, no soft-double.

Replay: put pinned original decrypted ARM64 and provided macho.py, Unicorn
and Capstone on the existing private workspace; python3 verify.py ARM64.
