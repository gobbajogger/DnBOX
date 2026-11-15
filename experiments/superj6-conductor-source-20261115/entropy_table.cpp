#include "entropy_table.hpp"
#include <algorithm>
namespace original_conductor_v1 {
namespace {
struct Reader {EntropyProvider provider;std::uint64_t budget,requests;EntropyStatus status;
 bool take(std::uint64_t& word)noexcept{
  if(requests==budget){status=EntropyStatus::BudgetExceeded;return false;}
  ++requests;if(!provider.next_word(provider.context,word)){status=EntropyStatus::SourceFailure;return false;}return true;
 }
};
// Exact 64x64 high half with 32-bit limbs; no compiler-specific __int128.
std::uint64_t high_product(std::uint64_t a,std::uint64_t b)noexcept{
 const std::uint64_t mask=0xffffffffULL,a0=a&mask,a1=a>>32,b0=b&mask,b1=b>>32;
 const std::uint64_t w0=a0*b0,t=a1*b0+(w0>>32),w1=(t&mask)+a0*b1;
 return a1*b1+(t>>32)+(w1>>32);
}
bool bounded32(Reader& r,std::uint32_t bound,std::uint32_t& out)noexcept{
 const std::uint32_t threshold=std::uint32_t(0U-bound)%bound;
 for(;;){std::uint64_t word;if(!r.take(word))return false;
  std::uint64_t product=std::uint64_t(std::uint32_t(word))*bound;
  if(std::uint32_t(product)>=threshold){out=std::uint32_t(product>>32);return true;}
 }
}
bool bounded64(Reader& r,std::uint64_t bound,std::uint64_t& out)noexcept{
 const std::uint64_t threshold=(std::uint64_t(0)-bound)%bound;
 for(;;){std::uint64_t word;if(!r.take(word))return false;
  if(word*bound>=threshold){out=high_product(word,bound);return true;}
 }
}
bool amplitude(Reader& r,float& out)noexcept{
 std::uint32_t x;if(!bounded32(r,16777217U,x))return false;
 if(x==16777216U){out=1.f;return true;}
 volatile float scaled=float(x)/16777216.f;volatile float doubled=scaled*2.f;out=doubled-1.f;return true;
}
}
EntropyResult build_entropy_table12(EntropyProvider provider,std::uint64_t budget,float (&output)[256]) noexcept{
 if(!provider.next_word)return {EntropyStatus::MissingProvider,0};
 Reader r{provider,budget,0,EntropyStatus::Complete};float temporary[256],discard;std::uint64_t duration;
 // Both initialization values are intentionally discarded, but their draws remain.
 if(!amplitude(r,discard)||!bounded64(r,207,duration))return {r.status,r.requests};
 std::uint64_t remaining=0;float value=0;
 for(unsigned i=0;i<256;i++){
  if(!remaining){if(!amplitude(r,value)||!bounded64(r,207,duration))return {r.status,r.requests};remaining=50+duration;}
  temporary[i]=value;--remaining;
 }
 std::copy(temporary,temporary+256,output);return {EntropyStatus::Complete,r.requests};
}
EntropyResult draw_held_sample(EntropyProvider provider,std::uint64_t budget,double& output) noexcept{
 if(!provider.next_word)return {EntropyStatus::MissingProvider,0};
 Reader r{provider,budget,0,EntropyStatus::Complete};std::uint64_t x;
 if(!bounded64(r,9007199254740993ULL,x))return {r.status,r.requests};
 double value=x==9007199254740992ULL?1.:(double(x)/9007199254740992.)*2.-1.;
 output=value;return {EntropyStatus::Complete,r.requests};
}

}
