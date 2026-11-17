#include "full_fixture.hpp"
#include "synthone_c.h"
int main(int argc,char**argv){CHECK(argc==2);Fixture f(argv[1]);const size_t n=dnbox_s1_owner_bytes();void*a=std::malloc(n);void*b=std::malloc(n);CHECK(a&&b);std::memset(a,0xa5,n);std::vector<unsigned char> before(n);std::memcpy(before.data(),a,n);
 FullPair t1(f,48000,false,false,0),t2(f,48000,false,false,0);
 for(unsigned i=0;i<52;i++){CHECK(t1.s.ft_array[i]==t1.s.ft_array[0]);CHECK(t2.s.ft_array[i]==t2.s.ft_array[0]);CHECK(t1.s.ft_array[i]!=t2.s.ft_array[i]);}
 for(unsigned i=0;i<52;i++){CHECK(t1.o.ft_array[i]->tbl==f.tables.morph[i].samples);if(i)CHECK(t1.o.ft_array[i]!=t1.o.ft_array[i-1]);}
 CHECK(t1.o.sine->tbl==f.tables.sine);
 CHECK(t1.s.sine!=t1.s.ft_array[0]);CHECK(t1.s.restore(t1.values,150));
 for(unsigned i=0;i<52;i++)CHECK(t1.s.ft_array[i]==t1.s.ft_array[0]);
 dnbox_s1_config c{nullptr,0,f.tables.sine,4097,f.tables.frequency_band,defaults,f.tuning,48000};
 CHECK(!dnbox_s1_init(a,n,&c));CHECK(!std::memcmp(a,before.data(),n));
 dnbox_s1_config bad=c;bad.compact=&c;bad.compact_bytes=1;CHECK(!dnbox_s1_init_harmonic(a,n,&bad));CHECK(!std::memcmp(a,before.data(),n));
 CHECK(!dnbox_s1_init_harmonic(a,n,nullptr));CHECK(!std::memcmp(a,before.data(),n));bad=c;bad.compact_bytes=1;CHECK(!dnbox_s1_init_harmonic(a,n,&bad));CHECK(!std::memcmp(a,before.data(),n));
 bad=c;CHECK(!dnbox_s1_init_harmonic_canonical_sine(a,n,&bad));CHECK(!std::memcmp(a,before.data(),n));
 bad.sine=nullptr;bad.sine_count=4096;CHECK(!dnbox_s1_init_harmonic_canonical_sine(a,n,&bad));CHECK(!std::memcmp(a,before.data(),n));
 bad.sine_count=4097;CHECK(!dnbox_s1_init_harmonic(a,n,&bad));CHECK(!std::memcmp(a,before.data(),n));
 dnbox_s1 *pa,*pb;{NoHeap guard;pa=dnbox_s1_init_harmonic(a,n,&c);pb=dnbox_s1_init_harmonic_canonical_sine(b,n,&bad);}CHECK(pa&&pb);CHECK(dnbox_s1_note_on(pa,60,100)&&dnbox_s1_note_on(pb,60,100));
 float al[128],ar[128],bl[128],br[128];for(int k=0;k<16;k++){NoHeap guard;CHECK(dnbox_s1_render(pa,128,al,ar));CHECK(dnbox_s1_render(pb,128,bl,br));CHECK(!std::memcmp(al,bl,sizeof al)&&!std::memcmp(ar,br,sizeof ar));}
 std::memcpy(before.data(),b,n);CHECK(dnbox_s1_note_off(pa,60));CHECK(!std::memcmp(b,before.data(),n));{NoHeap guard;CHECK(dnbox_s1_render(pa,128,al,ar));}CHECK(!std::memcmp(b,before.data(),n));
 {NoHeap guard;dnbox_s1_retire(pa);dnbox_s1_retire(pb);}std::free(a);std::free(b);std::puts("canonical API shallow rejection preserves storage; raw/canonical paired render and owner isolation passed; not complete admission/native fit");}
