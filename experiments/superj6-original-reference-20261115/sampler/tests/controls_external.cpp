/* INTERNAL REFERENCE COMPONENT, NOT A GUARDED PRODUCT API.
 * Only the documented external reference fixtures and caller lifetimes have
 * recorded-original comparison evidence. General finite input, descriptors,
 * concurrent use and lifetime misuse are not made safe here. See README.md.
 */
#include "CoreSampler.h"
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>
static unsigned readu(FILE*f){unsigned x;assert(fread(&x,4,1,f)==1);return x;}
// Explicit source-observed subset only. Ramped values are applied immediately here.
static bool apply(CoreSampler&s,unsigned id,float v){switch(id){
case 0:s.masterVolume=v;break;case 1:s.pitchOffset=v;break;case 2:s.vibratoDepth=v;break;case 3:s.vibratoFrequency=v;break;case 4:s.voiceVibratoDepth=v;break;case 5:s.voiceVibratoFrequency=v;break;case 6:s.cutoffMultiple=v;break;case 7:s.cutoffEnvelopeStrength=v;break;case 8:s.linearResonance=std::pow(10.0,-.05*double(v));break;case 9:s.glideRate=v;break;
case 10:s.setADSRAttackDurationSeconds(v);break;case 11:s.setADSRHoldDurationSeconds(v);break;case 12:s.setADSRDecayDurationSeconds(v);break;case 13:s.setADSRSustainFraction(v);break;case 14:s.setADSRReleaseHoldDurationSeconds(v);break;case 15:s.setADSRReleaseDurationSeconds(v);break;
case 16:s.setFilterAttackDurationSeconds(v);break;case 17:s.setFilterDecayDurationSeconds(v);break;case 18:s.setFilterSustainFraction(v);break;case 19:s.setFilterReleaseDurationSeconds(v);break;case 20:s.isFilterEnabled=v>.5f;break;case 21:s.restartVoiceLFO=v>.5f;break;
case 22:s.setPitchAttackDurationSeconds(v);break;case 23:s.setPitchDecayDurationSeconds(v);break;case 24:s.setPitchSustainFraction(v);break;case 25:s.setPitchReleaseDurationSeconds(v);break;case 26:s.pitchADSRSemitones=v;break;case 27:s.loopThruRelease=v>.5f;break;case 28:s.isMonophonic=v>.5f;break;case 29:s.isLegato=v>.5f;break;case 30:if(v!=0.f)return false;break; // source supports only sample-mode, not original oscillator mode
case 33:s.useJunoFilter=v>.5f;break;case 36:s.userCutoff=v;break;case 46:s.useSyrupFilter=v>.5f;break;case 47:s.syrup24=v>.5f;break;case 51:s.useOTAFilter=v>.5f;break;
case 39:s.keyTracking=v;break;case 40:s.filterEnvelopeVelocityScaling=v;break;default:return false;}return true;}
struct E{unsigned f,on,n,v;};int main(int ac,char**av){assert(ac==4);std::vector<float>sample(273015);FILE*f=fopen(av[1],"rb");assert(f&&fread(sample.data(),4,sample.size(),f)==sample.size());fclose(f);f=fopen(av[2],"rb");assert(f&&readu(f)==0x4a364331);unsigned count=readu(f);for(unsigned k=0;k<count;++k){unsigned frames=readu(f),np=readu(f),ne=readu(f);CoreSampler s;s.init(48000);SampleDataDescriptor d{};d.sampleDescriptor={60,261.6255653005986f,58,63,0,127,true,175902,272998,0,273014};d.sampleRate=44100;d.channelCount=1;d.sampleCount=273015;d.data=sample.data();s.loadSampleData(d);s.buildKeyMap();for(unsigned i=0;i<np;++i){unsigned id=readu(f);float v;assert(fread(&v,4,1,f)==1&&std::isfinite(v)&&apply(s,id,v));}std::vector<E>es(ne);for(auto&e:es){e.f=readu(f);e.on=readu(f);e.n=readu(f);e.v=readu(f);}char fn[512];snprintf(fn,sizeof fn,"%s/case-%02u-public.f32le.pcm",av[3],k);FILE*out=fopen(fn,"wb");assert(out);unsigned frame=0,ei=0;while(frame<frames){while(ei<es.size()&&es[ei].f==frame){auto&e=es[ei++];if(e.on)s.playNote(e.n,e.v);else s.stopNote(e.n,false);}unsigned n=std::min(16u,frames-frame);if(ei<es.size())n=std::min(n,es[ei].f-frame);assert(n);float l[16]={},r[16]={};float*p[]={l,r};s.render(2,n,p);for(unsigned j=0;j<n;++j){assert(fwrite(l+j,4,1,out)==1&&fwrite(r+j,4,1,out)==1);}frame+=n;}assert(fclose(out)==0);}fclose(f);}
