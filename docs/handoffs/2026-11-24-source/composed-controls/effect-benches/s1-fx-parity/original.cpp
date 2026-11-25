// Source-derived Synth One dry + original Soundpipe graph, not an iOS app capture.
#include "full_fixture.hpp"
#include "oracle_controls.hpp"
#define S1FX_EXISTING_PARAMETERS 1
#include "source_fx.hpp"
#include "stimulus.h"

struct Context { SourceFx*fx; float*l,*r; float mono; unsigned mode; };
static unsigned readu(FILE*f){unsigned v;CHECK(std::fread(&v,4,1,f)==1);return v;}
static unsigned getmode(const char*m){
      if(!std::strcmp(m,"wet"))return MODE_WET;
      if(!std::strcmp(m,"dry"))return MODE_DRY;
      if(!std::strcmp(m,"delay"))return MODE_DELAY;
      if(!std::strcmp(m,"reverb"))return MODE_REVERB;
      CHECK(false);return 0;
}
static void effect(const OracleState&s,unsigned phase,unsigned i,void*v) {
    if(phase!=1)return;
    auto&ctx=*static_cast<Context*>(v);auto&f=*ctx.fx;
    if(ctx.mono!=s.parameters[isMono]) {ctx.mono=s.parameters[isMono];f.source_reset_delays();}
    std::memcpy(f.parameters.data(),s.parameters,150*4);
    if(ctx.mode!=MODE_WET) {
        f.parameters[bitCrushSampleRate]=S1AB_RATE;
        f.parameters[bitcrushLFO]=0.f; f.parameters[tremoloLFO]=0.f;
        f.parameters[autoPanAmount]=0.f; f.parameters[phaserMix]=0.f; f.parameters[widen]=0.f;
        if(ctx.mode!=MODE_DELAY) {f.parameters[delayOn]=0.f;f.parameters[delayMix]=0.f;}
        if(ctx.mode!=MODE_REVERB) {f.parameters[reverbOn]=0.f;f.parameters[reverbMix]=0.f;}
    }
    f.frame(ctx.l[i],ctx.l+i,ctx.r+i,s.lfo1_0_1,s.lfo1_1_0,s.lfo2_0_1,s.lfo2_1_0);
}
static void render(FullOracle&o,Context&c,unsigned samples,FILE*out) {
    while(samples) {
        float l[S1AB_CHUNK]{},r[S1AB_CHUNK]{},st[2*S1AB_CHUNK];
        auto n=std::min(samples,S1AB_CHUNK);samples-=n;
        c.l=l;c.r=r;
        o.trace_fn=effect;o.trace_context=&c;
        compact_oracle_window=true;o.process(n,0,l,r);compact_oracle_window=false;
        o.trace_fn=nullptr;
        for(unsigned i=0;i<n;++i) {CHECK(std::isfinite(l[i]) && std::isfinite(r[i]));st[2*i]=l[i];st[2*i+1]=r[i];}
        if(out)CHECK(std::fwrite(st,sizeof(float)*2,n,out)==n);
    }
}
int main(int argc,char**argv) {
    CHECK(argc==6);
    Fixture fixture(argv[1]);
    FILE*f=std::fopen(argv[2],"rb");CHECK(f && readu(f)==0x53315043);
    unsigned want=(unsigned)std::strtoul(argv[3],nullptr,10), count=readu(f);CHECK(want<count);
    FullOracle o;
    auto vals=defaults; // explicit initialization below to mirror FullPair's control start
    float init[150];
    std::memcpy(init,vals,sizeof init);
    init[isMono]=0;init[arpIsOn]=1;init[arpIsSequencer]=0;init[arpDirection]=0;
    init[arpRate]=200;init[arpSeqTempoMultiplier]=1.f/64.f/1.5f;init[arpOctave]=3;init[arpTotalSteps]=16;
    for(int i=0;i<16;i++){init[sequencerPattern00+i]=(i%13)*(i%2?-1:1);init[sequencerNoteOn00+i]=i%3?1:0;init[sequencerOctBoost00+i]=i%2;}
    CHECK(o.init(fixture.tables,S1AB_RATE,init,150,fixture.tuning,128));
    s1frontend_oracle::OracleControls ctrl(o);
    for(unsigned row=0;row<=want;++row){
        unsigned n=readu(f);CHECK(n==144);
        for(unsigned j=0;j<n;++j){unsigned id=readu(f);float val;CHECK(std::fread(&val,4,1,f)==1);if(row==want)CHECK(ctrl.set_parameter(id,val));}
    }
    CHECK(!std::fclose(f));o.resetSequencer();
    SourceFx fx(S1AB_RATE);
    Context ctx{&fx,nullptr,nullptr,0.f,getmode(argv[5])};
    FILE*out=std::fopen(argv[4],"wb");CHECK(out);
    render(o,ctx,S1AB_LEAD,nullptr);
    for(unsigned i=0;i<S1AB_NOTES;++i) {
        o.startNote(s1ab_note[i],s1ab_vel(i));
        render(o,ctx,S1AB_NOTE_LEN,out);
        o.stopNote(s1ab_note[i]);
    }
    render(o,ctx,S1AB_TAIL,out);
    CHECK(!std::fclose(out));
    std::printf("original %u %s %u frames, delay on %.6g mix %.6g, reverb on %.6g mix %.6g\n",want,argv[5],S1AB_FRAMES,
       fx.parameters[delayOn],fx.parameters[delayMix],fx.parameters[reverbOn],fx.parameters[reverbMix]);
}
