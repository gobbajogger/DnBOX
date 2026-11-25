#include "app/original_runtime.h"
#include "app/x0x.h"
#define O(n,t,x) const unsigned n=__builtin_offsetof(t,x)
const unsigned partBytes=sizeof(original_runtime_part);
O(gateCfg,original_runtime_part,arp.cfg.gate);
O(gateTicks,original_runtime_part,arp.prm.gateTicks);
O(latchTicks,original_runtime_part,arp.seq.liveGateTicks);
O(beatTick,original_runtime_part,arp.seq.mBeatTime);
O(gateOpen,original_runtime_part,arp.seq.gateOpen);
O(noteCount,original_runtime_part,arp.seq.sequencerLastNotesCount);
O(arpKind,original_runtime_part,kind);
O(arpDiv,original_runtime_part,arp.cfg.division);
O(drumCh,seq_t,ch);
O(sound,project_t,sound);
