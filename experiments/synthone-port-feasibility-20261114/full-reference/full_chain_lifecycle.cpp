#include "full_fixture.hpp"
#define S1FX_EXISTING_PARAMETERS 1
#include "source_fx.hpp"
struct FxContext{SourceFx*fx;float*l,*r;unsigned offset;float previous_mode;};
static void apply_fx(SourceFx&f,const float*p,float in,float*l,float*r,float a,float b,float c,float d){std::memcpy(f.parameters.data(),p,150*sizeof(float));f.frame(in,l,r,a,b,c,d);}
static void candidate_fx(const RuntimeState&s,unsigned phase,unsigned frame,void*v){if(phase!=1)return;auto&c=*static_cast<FxContext*>(v);if(c.previous_mode!=s.parameters[isMono]){c.previous_mode=s.parameters[isMono];c.fx->source_reset_delays();}unsigned i=c.offset+frame;apply_fx(*c.fx,s.parameters,c.l[i],c.l+i,c.r+i,s.lfo1_0_1,s.lfo1_1_0,s.lfo2_0_1,s.lfo2_1_0);}
static void original_fx(const OracleState&s,unsigned phase,unsigned frame,void*v){if(phase!=1)return;auto&c=*static_cast<FxContext*>(v);if(c.previous_mode!=s.parameters[isMono]){c.previous_mode=s.parameters[isMono];sp_vdelay_reset(c.fx->sp,c.fx->delayL);sp_vdelay_reset(c.fx->sp,c.fx->delayR);sp_vdelay_reset(c.fx->sp,c.fx->delayRR);sp_vdelay_reset(c.fx->sp,c.fx->delayFillIn);}unsigned i=c.offset+frame;apply_fx(*c.fx,s.parameters,c.l[i],c.l+i,c.r+i,s.lfo1_0_1,s.lfo1_1_0,s.lfo2_0_1,s.lfo2_1_0);}
int main(int argc,char**argv){
 CHECK(argc==4);Fixture f(argv[1]);FILE*file=std::fopen(argv[2],"rb");CHECK(file);std::vector<unsigned char>pack(165292);CHECK(std::fread(pack.data(),1,pack.size(),file)==pack.size());CHECK(!std::fclose(file));compact_data=pack.data();compact_size=pack.size();
 unsigned scene=0;unsigned long long total=0;double maxdelta=0;
 for(bool mono:{false,true})for(bool seq:{false,true})for(int direction:{0,1,2}){
  case_name="Full original seq/arp+compact reader through source-authentic fullFX shared reference";FullPair p(f,48000,mono,seq,direction);SourceFx a(48000),b(48000);
  p.parameter(delayTime,.05f);p.parameter(delayMix,.4f);p.parameter(delayOn,1);p.parameter(reverbMix,.25f);p.parameter(reverbOn,1);p.parameter(phaserMix,.3f);p.parameter(widen,1);p.parameter(autoPanAmount,.6f);p.parameter(autoPanFrequency,3.f);p.parameter(bitcrushLFO,3);p.parameter(tremoloLFO,2);p.parameter(reverbMixLFO,1);p.parameter(lfo1Amplitude,.7f);p.parameter(lfo2Amplitude,.8f);p.parameter(lfo1Rate,4.f);p.parameter(lfo2Rate,6.f);p.start(60,90);p.start(67,127);p.start(72,31);
  char sa[512],sb[512];std::snprintf(sa,sizeof sa,"%s/scene-%02u-original.f32le-stereo.pcm",argv[3],scene);std::snprintf(sb,sizeof sb,"%s/scene-%02u-compact.f32le-stereo.pcm",argv[3],scene);FILE*oa=std::fopen(sa,"wb"),*ob=std::fopen(sb,"wb");CHECK(oa&&ob);
  double energy=0;float peak=0;unsigned nframe=0;FxContext sc{&a,nullptr,nullptr,0,p.s.parameters[isMono]},oc{&b,nullptr,nullptr,0,p.o.parameters[isMono]};
  while(nframe<48000){unsigned n=std::min(128u,48000-nframe);float sl[128]{},sr[128]{},ol[128]{},orr[128]{};sc.l=sl;sc.r=sr;oc.l=ol;oc.r=orr;p.s.trace_fn=candidate_fx;p.s.trace_context=&sc;p.o.trace_fn=original_fx;p.o.trace_context=&oc;
   compact_oracle_window=true;p.o.process(n,0,ol,orr);compact_oracle_window=false;{NoHeap guard;CHECK(p.s.process(n,0,sl,sr));}p.s.trace_fn=nullptr;p.o.trace_fn=nullptr;p.s.trace_context=p.o.trace_context=nullptr;
   CHECK(!std::memcmp(sl,ol,n*sizeof(float))&&!std::memcmp(sr,orr,n*sizeof(float)));
   for(unsigned i=0;i<n;++i){CHECK(std::isfinite(sl[i])&&std::isfinite(sr[i]));float rawA[2]={ol[i],orr[i]},rawB[2]={sl[i],sr[i]};CHECK(std::fwrite(rawA,sizeof(float),2,oa)==2&&std::fwrite(rawB,sizeof(float),2,ob)==2);energy+=double(sl[i])*sl[i]+double(sr[i])*sr[i];peak=std::max(peak,std::max(std::fabs(sl[i]),std::fabs(sr[i])));}
   nframe+=n;total+=n;
   if(nframe==12032){p.parameter(isMono,mono?0:1);p.stop(67);p.start(48,64);p.parameter(phaserMix,0);p.parameter(reverbMix,.6f);}if(nframe==24064){p.reset_dsp();a.source_reset_delays();sp_vdelay_reset(b.sp,b.delayL);sp_vdelay_reset(b.sp,b.delayR);sp_vdelay_reset(b.sp,b.delayRR);sp_vdelay_reset(b.sp,b.delayFillIn);p.parameter(arpIsOn,1);p.start(55,110);p.parameter(delayTime,.1f);p.parameter(bitCrushSampleRate,4096);}if(nframe==36096){p.restore();std::memcpy(a.parameters.data(),p.s.parameters,150*4);std::memcpy(b.parameters.data(),p.o.parameters,150*4);a.source_restore_effect_controls();b.source_restore_effect_controls();sc.previous_mode=p.s.parameters[isMono];oc.previous_mode=p.o.parameters[isMono];p.start(62,101);p.start(65,82);}
  }
  CHECK(!std::fclose(oa)&&!std::fclose(ob));CHECK(energy>0);p.compare();std::printf("{\"scene\":%u,\"mono\":%s,\"sequence\":%s,\"direction\":%d,\"frames\":48000,\"bitexact\":true,\"peak\":%.9g,\"energy\":%.17g}\n",scene,mono?"true":"false",seq?"true":"false",direction,peak,energy);std::fflush(stdout);++scene;
 }
 std::printf("{\"scope\":\"host paired original-note/fullseq+common authentic fullFX with mono-switch/reset/hot-restore lifecycle; not independent targetFX or device fidelity\",\"scenes\":%u,\"frames\":%llu,\"max_abs_delta\":%.17g,\"all_finite\":true,\"device_or_targetCPU_proof\":false}\n",scene,total,maxdelta);
}
