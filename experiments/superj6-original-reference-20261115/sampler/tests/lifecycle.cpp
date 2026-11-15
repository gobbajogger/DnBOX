#include "SamplerVoice.h"
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace DunneCore;
int main(){
 AHDSHREnvelopeParameters amp;ADSREnvelopeParameters filter,pitch;
 amp.init(48000,.01,0,.05,.7,0,.1);filter.init(48000,.02,.1,.7,.2);pitch.init(48000,.03,.2,.5,.3);
 for(int operation=0;operation<3;++operation){
 SamplerVoice v;v.ampEnvelope.pParameters=&amp;v.filterEnvelope.pParameters=&filter;v.pitchEnvelope.pParameters=&pitch;v.init(48000);float glide=0;v.glideSecPerOctave=&glide;v.glideSemitones=0;v.noteFrequency=261.6255653005986f;
 SampleBuffer buffer;buffer.sampleRate=44100;buffer.noteFrequency=261.6255653005986f;v.sampleBuffer=&buffer;
 v.leftSmoothFilter.setParameters(1700,1.3);v.rightSmoothFilter.setParameters(1300,.8);
 for(int i=0;i<64;++i){v.leftSmoothFilter.process(.2f+float(i)*.001f);v.rightSmoothFilter.process(-.1f-float(i)*.002f);}
 auto l=v.leftSmoothFilter,r=v.rightSmoothFilter;
 if(l.first||r.first||l.x1==0||r.x1==0)return 2;
 if(operation==0)v.start(60,96000,261.6255653005986f,.75f,&buffer);
 else if(operation==1)v.restartNewNote(60,96000,261.6255653005986f,.75f,&buffer);
 else v.restartNewNoteLegato(60,96000,261.6255653005986f);
 auto retained=[](const OriginalSmoothPassFilter&a,const OriginalSmoothPassFilter&b){return a.x1==b.x1&&a.x2==b.x2&&a.y1==b.y1&&a.y2==b.y2&&a.first==b.first&&std::memcmp(a.c,b.c,sizeof a.c)==0&&a.cutoff==b.cutoff&&a.resonance==b.resonance;};
 if(!retained(l,v.leftSmoothFilter)||!retained(r,v.rightSmoothFilter)){std::fprintf(stderr,"FAIL original voice rate update reset SmoothPass histories/current coefficients/first/parameters\n");return 1;}
 if(v.leftSmoothFilter.rate!=96000||v.rightSmoothFilter.rate!=96000||v.leftSmoothFilter.smoothing==l.smoothing||std::memcmp(v.leftSmoothFilter.target,l.target,sizeof l.target)==0)return 3;
 }
 std::puts("PASS three original-compatible start/restart/legato rate updates preserves histories/current coefficients/first/parameters and updates rate/smoothing/targets; own-source regression, not original execution");
}
