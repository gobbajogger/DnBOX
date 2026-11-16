// Actual source-derived FullOracle versus the public pre-FX renderer adapter.
// No preset bank/full FX/iOS/device identity is claimed by this host test.
#define main retained_source_scheduler_test_main
#include "tests/compact_scheduler_test.cpp"
#undef main
#include "source_scheduler.hpp"
#include "full_oracle.hpp"
#include "synthone_renderer.hpp"
#include "synthone_c.h"
int main(int argc,char **argv) {
 CHECK(argc==4); Fixture f(argv[1]); FILE *packfile=std::fopen(argv[2],"rb"); CHECK(packfile);
 std::vector<unsigned char> pack(165292);CHECK(std::fread(pack.data(),1,pack.size(),packfile)==pack.size());CHECK(!std::fclose(packfile));
 unsigned long long count=0, different=0, nonzero=0; float peak=0;
 FILE *audio=std::fopen(argv[3],"wb");CHECK(audio);
 for(int rate:{44100,48000,96000}) for(bool mono:{false,true}) for(bool seq:{false,true}) {
  dnbox::SynthOneRenderer s; FullOracle o; float v[150];std::memcpy(v,defaults,sizeof v);
  v[isMono]=mono;v[arpIsOn]=seq;v[arpIsSequencer]=seq;v[arpRate]=120;v[arpSeqTempoMultiplier]=.25f;
  v[arpTotalSteps]=4;for(unsigned k=0;k<16;k++){v[sequencerPattern00+k]=(k%4)*3;v[sequencerNoteOn00+k]=1;}
  Tables metadata=f.tables;for(auto &row:metadata.morph)row.samples=nullptr;
  std::vector<unsigned char> raw(dnbox_s1_owner_bytes()+dnbox_s1_owner_alignment());
  uintptr_t address=(reinterpret_cast<uintptr_t>(raw.data())+dnbox_s1_owner_alignment()-1)&~(dnbox_s1_owner_alignment()-1);
  void *storage=reinterpret_cast<void*>(address);
  dnbox_s1_config config{pack.data(),pack.size(),f.tables.sine,f.tables.sine_count,f.tables.frequency_band,v,f.tuning,rate};
  CHECK(!dnbox_s1_init(nullptr,dnbox_s1_owner_bytes(),&config));
  CHECK(!dnbox_s1_init(storage,dnbox_s1_owner_bytes()-1,&config));
  CHECK(!dnbox_s1_init(static_cast<unsigned char*>(storage)+1,dnbox_s1_owner_bytes(),&config));
  CHECK(!dnbox_s1_init(storage,dnbox_s1_owner_bytes(),reinterpret_cast<const dnbox_s1_config*>(storage)));
  dnbox_s1_config bad=config;bad.parameters=static_cast<float*>(storage);
  CHECK(!dnbox_s1_init(storage,dnbox_s1_owner_bytes(),&bad));
  dnbox_s1 *handle=dnbox_s1_init(storage,dnbox_s1_owner_bytes(),&config);CHECK(handle);
  CHECK(!dnbox_s1_parameter(handle,150,0));CHECK(!dnbox_s1_parameter(handle,0,NAN));
  CHECK(!dnbox_s1_note_on(handle,128,100));CHECK(!dnbox_s1_note_on(handle,60,128));
  CHECK(dnbox_s1_note_on(handle,60,100));CHECK(dnbox_s1_note_on(handle,67,90));
  CHECK(s.prepare(metadata,pack.data(),pack.size(),rate,v,150,f.tuning,128));CHECK(o.init(f.tables,rate,v,150,f.tuning,128));
  CHECK(s.note_on(60,100));o.startNote(60,100);CHECK(s.note_on(67,90));o.startNote(67,90);
  for(unsigned b=0;b<375;b++) {
   if(b==125){CHECK(dnbox_s1_note_off(handle,67));CHECK(s.note_off(67));o.stopNote(67);}
   if(b==250){CHECK(dnbox_s1_note_off(handle,60));CHECK(s.note_off(60));o.stopNote(60);}
   float sl[128]{},sr[128]{},ol[128]{},orr[128]{},cl[128]{},cr[128]{};
   {NoHeap g;CHECK(dnbox_s1_render(handle,128,cl,cr));CHECK(s.render(128,sl,sr));compact_oracle_window=true;o.process(128,0,ol,orr);compact_oracle_window=false;}
   for(unsigned i=0;i<128;i++){
    CHECK(!std::memcmp(cl+i,sl+i,4)&&!std::memcmp(cr+i,sr+i,4));
    CHECK(std::isfinite(sl[i])&&std::isfinite(sr[i]));different+=(std::memcmp(sl+i,ol+i,4)!=0);different+=(std::memcmp(sr+i,orr+i,4)!=0);
    nonzero+=(sl[i]!=0);peak=std::max(peak,std::fabs(sl[i]));count++;
    if(rate==48000&&!mono&&!seq){float stereo[2]={sl[i],sr[i]};CHECK(std::fwrite(stereo,sizeof(float),2,audio)==2);}
   }
  }
  CHECK(dnbox_s1_all_off(handle));CHECK(dnbox_s1_reset(handle));dnbox_s1_retire(handle);
 }
 CHECK(!std::fclose(audio));std::printf("{\"scope\":\"new-adapter-vs-actual-source-pre-FX\",\"scenes\":12,\"frames\":%llu,\"different_channel_words\":%llu,\"nonzero_left_frames\":%llu,\"peak\":%.9g,\"heap_calls_during_render\":0}\n",count,different,nonzero,peak);
 return different||!nonzero;
}
