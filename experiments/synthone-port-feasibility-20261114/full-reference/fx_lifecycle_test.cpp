#include "source_fx.hpp"
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>
static void manual_reset(SourceFx&f){
    sp_vdelay_reset(f.sp, f.delayL);
    sp_vdelay_reset(f.sp, f.delayR);
    sp_vdelay_reset(f.sp, f.delayRR);
    sp_vdelay_reset(f.sp, f.delayFillIn);
}
static void manual_restore(SourceFx&f){
    *f.phaser0->MinNotch1Freq = 100;
    *f.phaser0->MaxNotch1Freq = 800;
    *f.phaser0->Notch_width = 1000;
    *f.phaser0->NotchFreq = 1.5;
    *f.phaser0->VibratoMode = 1;
    *f.phaser0->depth = 1;
    *f.phaser0->feedback_gain = 0;
    *f.phaser0->invert = 0;
    *f.phaser0->lfobpm = 30;

    f.loPassInputDelayL->freq = f.getSynthParameter(cutoff);
    f.loPassInputDelayL->res = f.getSynthParameter(delayInputResonance);
    f.loPassInputDelayR->freq = f.getSynthParameter(cutoff);
    f.loPassInputDelayR->res = f.getSynthParameter(delayInputResonance);


}
int main(){unsigned long long frames=0;unsigned resets=0,restores=0;
for(int rate:{44100,48000}){SourceFx a(rate),b(rate);a.parameters[delayTime]=b.parameters[delayTime]=.005f;a.parameters[delayOn]=b.parameters[delayOn]=1;a.parameters[delayMix]=b.parameters[delayMix]=.7f;a.parameters[reverbOn]=b.parameters[reverbOn]=1;a.parameters[reverbMix]=b.parameters[reverbMix]=.5f;a.parameters[phaserMix]=b.parameters[phaserMix]=.3f;a.parameters[widen]=b.parameters[widen]=1;
 for(unsigned i=0;i<60000;++i){
  if(i==12000||i==36000){float prev[]={a.delayL->prev,a.delayR->prev,a.delayRR->prev,a.delayFillIn->prev};a.source_reset_delays();manual_reset(b);sp_vdelay*q[]={a.delayL,a.delayR,a.delayRR,a.delayFillIn};for(unsigned k=0;k<4;++k){assert(!memcmp(&q[k]->prev,&prev[k],4));assert(q[k]->left==0);for(size_t n=0;n<q[k]->buf.size/4;++n)assert(((float*)q[k]->buf.ptr)[n]==0.f);}++resets;}
  if(i==24000||i==48000){*a.phaser0->MinNotch1Freq=*b.phaser0->MinNotch1Freq=199;a.parameters[cutoff]=b.parameters[cutoff]=1234;a.source_restore_effect_controls();manual_restore(b);++restores;}
  float x=i<30000?.15f*std::sin(float(i)*.031f):0.f,al,ar,bl,br;a.frame(x,&al,&ar,.3f,.8f,.6f,.9f);b.frame(x,&bl,&br,.3f,.8f,.6f,.9f);assert(std::isfinite(al)&&std::isfinite(ar));assert(!memcmp(&al,&bl,4)&&!memcmp(&ar,&br,4));++frames;
 }
}
printf("{\"frames\":%llu,\"source_reset_calls\":%u,\"source_restore_calls\":%u,\"bitexact_manual_original_calls\":true,\"scope\":\"SourceFX component lifecycle, not complete app or device\"}\n",frames,resets,restores);
}
