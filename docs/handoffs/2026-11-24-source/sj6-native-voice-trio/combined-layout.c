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
O(mono,original_runtime_part,dsp.juno.synth.mono32_on);
O(legato,original_runtime_part,dsp.juno.synth.legato33_on);
O(glide,original_runtime_part,dsp.juno.synth.glide35_detent);
O(glideOffset,original_runtime_part,dsp.juno.synth.core.sj6_glide_offset);
O(voice0note,original_runtime_part,dsp.juno.synth.core.voice[0].note);
O(voice0gate,original_runtime_part,dsp.juno.synth.core.voice[0].gate);
O(voice0velocity,original_runtime_part,dsp.juno.synth.core.voice[0].velocity);
O(voiceB0,project_t,sound.v[T_303][9][4]);
O(voiceB1,project_t,sound.v[T_303][11][4]);
O(sndRev,project_t,set.snd_rev);
