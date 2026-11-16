// Actual source-derived FullOracle versus the public pre-FX renderer adapter.
// No preset bank/full FX/iOS/device identity is claimed by this host test.
#define main retained_source_scheduler_test_main
#include "tests/compact_scheduler_test.cpp"
#undef main
#include "source_scheduler.hpp"
#include "full_oracle.hpp"
#include "synthone_renderer.hpp"
#include "synthone_c.h"
#include "oracle_controls.hpp"
static void test_source_frontend(Fixture&f,const std::vector<unsigned char>&pack) {
 FullOracle original;CHECK(original.init(f.tables,48000,defaults,150,f.tuning,128));
 s1frontend_oracle::OracleControls reference(original);
 std::vector<unsigned char> raw(dnbox_s1_owner_bytes()+dnbox_s1_owner_alignment());
 uintptr_t address=(reinterpret_cast<uintptr_t>(raw.data())+dnbox_s1_owner_alignment()-1)&~(dnbox_s1_owner_alignment()-1);
 void *storage=reinterpret_cast<void*>(address);
 dnbox_s1_config config{pack.data(),pack.size(),f.tables.sine,f.tables.sine_count,f.tables.frequency_band,defaults,f.tuning,48000};
 dnbox_s1*handle=dnbox_s1_init(storage,dnbox_s1_owner_bytes(),&config);CHECK(handle);
 unsigned changes=0;
 auto compare=[&](){for(unsigned id=0;id<150;id++){float actual;CHECK(dnbox_s1_target(handle,id,&actual));float expected=reference.target(id);CHECK(!std::memcmp(&actual,&expected,sizeof actual));}};
 auto physical=[&](unsigned id,float x){{NoHeap guard;CHECK(dnbox_s1_source_parameter(handle,id,x));}CHECK(reference.set_parameter(id,x));compare();++changes;};
 for(unsigned id=0;id<150;++id)physical(id,defaults[id]);
 for(int tempo:{20,60,120,200,300})for(float sync:{0.f,1.f}) {
  physical(arpRate,tempo);physical(tempoSyncToArpRate,sync);
  for(unsigned id:{unsigned(lfo1Rate),unsigned(lfo2Rate),unsigned(autoPanFrequency),unsigned(delayTime),unsigned(arpSeqTempoMultiplier),unsigned(pitchbend)})for(unsigned k=0;k<=100;++k){
   {NoHeap guard;CHECK(dnbox_s1_source_normalized(handle,id,k/100.f));}
   CHECK(reference.set_normalized(id,k/100.f,0));compare();++changes;
  }
 }
 physical(frequencyA4,442.9f);float a4;CHECK(dnbox_s1_target(handle,frequencyA4,&a4));CHECK(a4==442.f);
 std::vector<unsigned char> before(dnbox_s1_owner_bytes());std::memcpy(before.data(),storage,before.size());
 CHECK(!dnbox_s1_source_parameter(nullptr,0,1));CHECK(!dnbox_s1_source_parameter(handle,150,0));CHECK(!dnbox_s1_source_parameter(handle,0,NAN));
 CHECK(!dnbox_s1_source_normalized(nullptr,lfo1Rate,.5f));CHECK(!dnbox_s1_source_normalized(handle,150,.5f));
 CHECK(!dnbox_s1_source_normalized(handle,0,.5f));CHECK(!dnbox_s1_source_normalized(handle,lfo1Rate,-.01f));
 CHECK(!dnbox_s1_source_normalized(handle,lfo1Rate,1.01f));CHECK(!dnbox_s1_source_normalized(handle,lfo1Rate,INFINITY));
 float output=123.25f;CHECK(!dnbox_s1_target(nullptr,0,&output));CHECK(!dnbox_s1_target(handle,150,&output));CHECK(output==123.25f);
 CHECK(!dnbox_s1_target(handle,0,nullptr));CHECK(!dnbox_s1_target(handle,0,static_cast<float*>(storage)));
 CHECK(!dnbox_s1_target(handle,0,reinterpret_cast<float*>(static_cast<unsigned char*>(storage)+1)));
 CHECK(!std::memcmp(before.data(),storage,before.size()));compare();
 // This checks actual setter-to-render integration, not just returned targets.
 for(unsigned id=0;id<150;++id)physical(id,defaults[id]);
 physical(arpIsOn,0);physical(isMono,0);physical(lfo1Amplitude,.25f);physical(lfo1Rate,3.5f);
 CHECK(dnbox_s1_note_on(handle,60,100));original.startNote(60,100);
 unsigned different=0;
 for(unsigned block=0;block<32;block++) {
  float left[128]{},right[128]{},ol[128]{},orr[128]{};
  {NoHeap guard;CHECK(dnbox_s1_render(handle,128,left,right));compact_oracle_window=true;original.process(128,0,ol,orr);compact_oracle_window=false;}
  for(unsigned i=0;i<128;i++){CHECK(std::isfinite(left[i])&&std::isfinite(right[i]));different+=std::memcmp(left+i,ol+i,4)!=0;different+=std::memcmp(right+i,orr+i,4)!=0;}
 }
 CHECK(different==0);dnbox_s1_retire(handle);
 std::printf("{\"source_frontend_changes\":%u,\"target_word_comparisons\":%u,\"render_frames\":4096,\"different_words\":%u,\"control_heap_calls\":0,\"scope\":\"original scalar helper laws only\"}\n",changes,changes*150,different);
}

int main(int argc,char **argv) {
 CHECK(dnbox_s1_owner_bytes()<=9600); // intermediate host owner budget; not target fit
 CHECK(s1scheduler::NoteNumber(-256,0,100).noteNumber==255);
 CHECK(s1scheduler::NoteNumber(60,0,256).noteNumber==255);
 CHECK(s1scheduler::NoteNumber(60,65536,100).noteNumber==255);
 for(int note=0;note<128;note++) for(int vel=0;vel<128;vel++) for(int trans=-24;trans<=24;trans++) {
  const s1scheduler::NoteNumber n(note,trans,vel);CHECK(n.valid());CHECK(n.noteNumber==note&&n.velocity==vel&&n.transpose==trans);
 }
 s1scheduler::Held held;unsigned char saved[sizeof held];std::memcpy(saved,&held,sizeof held);
 CHECK(!held.insert_checked(s1scheduler::NoteNumber(-256,0,100),0));
 CHECK(!std::memcmp(saved,&held,sizeof held));held.insert(s1scheduler::NoteNumber(60,65536,100),0);
 CHECK(!std::memcmp(saved,&held,sizeof held));
 CHECK(argc==4); Fixture f(argv[1]); FILE *packfile=std::fopen(argv[2],"rb"); CHECK(packfile);
 std::vector<unsigned char> pack(165292);CHECK(std::fread(pack.data(),1,pack.size(),packfile)==pack.size());CHECK(!std::fclose(packfile));
 test_source_frontend(f,pack);
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
