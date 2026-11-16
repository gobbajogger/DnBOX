// Scheduled arp witness: the compact reader is really consumed on an arp-on path.
// Positive: 52 NULL runtime morph payloads, compact reader calls, >=8192 nonzero frames
// bit-exact to the original FullOracle. Negative control: a finite-wrong compact byte that
// still passes structural admission must change finite PCM (so the reader is not unused).
#include "full_fixture.hpp"
static void scene(FullPair&p){p.start(60,90);p.start(67,127);p.start(72,31);}
int main(int argc,char**argv){
 CHECK(argc==3);Fixture f(argv[1]);FILE*file=std::fopen(argv[2],"rb");CHECK(file);
 std::vector<unsigned char>pack(165292);CHECK(std::fread(pack.data(),1,pack.size(),file)==pack.size());CHECK(!std::fclose(file));
 compact_data=pack.data();compact_size=pack.size();
 const unsigned rate=48000,block=128,target=8192,cap=rate*8;
 std::vector<float> ref;unsigned nonzero=0,frames_total=0,null_payloads=0;
 {
  case_name="scheduled arp witness, compact vs original";FullPair p(f,rate,false,false,0);
  for(unsigned i=0;i<52;i++)if(!p.s.ft_array[i]->tbl)null_payloads++;
  CHECK(null_payloads==52);scene(p);const unsigned long long reads0=compact_word_reads;
  while(nonzero<target&&frames_total<cap){
   float sl[block],sr[block],ol[block],orr[block];
   compact_oracle_window=true;p.o.process(block,0,ol,orr);compact_oracle_window=false;
   {NoHeap guard;CHECK(p.s.process(block,0,sl,sr));}
   CHECK(!std::memcmp(sl,ol,sizeof sl)&&!std::memcmp(sr,orr,sizeof sr));
   for(unsigned i=0;i<block;i++){CHECK(std::isfinite(ol[i])&&std::isfinite(orr[i]));if(ol[i]!=0.f||orr[i]!=0.f)nonzero++;ref.push_back(ol[i]);ref.push_back(orr[i]);}
   frames_total+=block;
  }
  p.compare();CHECK(nonzero>=target);
  std::printf("{\"witness\":\"positive\",\"rate\":%u,\"frames\":%u,\"nonzero_frames\":%u,\"null_runtime_payloads\":%u,\"compact_reader_calls\":%llu,\"bitexact\":true}\n",rate,frames_total,nonzero,null_payloads,compact_word_reads-reads0);
 }
 // Negative control: every compact word read returns with its LSB flipped (finite, wrong).
 bool found=false;
 {
  case_name="finite-wrong compact control";compact_wrong_lsb=true;FullPair q(f,rate,false,false,0);scene(q);
  unsigned changed=0;bool finite=true;
  for(unsigned done=0;done<frames_total;done+=block){
   float sl[block],sr[block];{NoHeap guard;CHECK(q.s.process(block,0,sl,sr));}
   for(unsigned i=0;i<block;i++){finite=finite&&std::isfinite(sl[i])&&std::isfinite(sr[i]);
    float a=ref[2*(done+i)],c=ref[2*(done+i)+1];if(std::memcmp(&a,&sl[i],4)||std::memcmp(&c,&sr[i],4))changed++;}
  }
  compact_wrong_lsb=false;found=finite&&changed>0;
  std::printf("{\"witness\":\"negative-control\",\"kind\":\"lsb-flip on every compact word read\",\"finite\":%s,\"changed_frames\":%u,\"of_frames\":%u}\n",finite?"true":"false",changed,frames_total);
 }
 compact_data=pack.data();CHECK(found);
 std::printf("{\"scope\":\"arp witness\",\"failures\":0}\n");return 0;
}
