// ORIGINAL event behavior adapted from the pinned MIT SynthOne source.
// ONE serialized audio owner; all control/getter/render calls are serialized.
// Fixed caller-owned representation, not DnBOX sequencing or an app-fidelity claim.
// No concurrent UI access/atomic IRQ shim; see SOURCE-SEQUENCE-CONTRACT.md.
#pragma once
#include "sequence_types.hpp"
#if !defined(S1_NO_DOUBLE)
#include <math.h>
#endif
#include <stdint.h>
#include <string.h>
#if defined(S1_NO_DOUBLE)
/* Beat clock in units of 2^-64. Addition is an integer ratio, not a soft double.
 * Round-to-nearest stays within about 5e-11 of the original binary64 sum on the
 * public sequence patterns. A step decision can differ by one sample when the
 * binary64 sum sits closer to a multiplier boundary than that residual. */
struct S1Clock { uint64_t whole, frac; int ok; };
struct S1U128 { uint64_t hi, lo; };
static inline int s1u_ge(S1U128 a, S1U128 b) { return a.hi>b.hi || (a.hi==b.hi && a.lo>=b.lo); }
static inline int s1u_lt(S1U128 a, S1U128 b) { return !s1u_ge(a,b); }
static inline S1U128 s1u_sub(S1U128 a, S1U128 b) { S1U128 r; r.lo=a.lo-b.lo; r.hi=a.hi-b.hi-(a.lo<b.lo); return r; }
static inline S1U128 s1u_shl1(S1U128 a) { return {(a.hi<<1)|(a.lo>>63), a.lo<<1}; }
static inline void s1u_divmod(S1U128 n, S1U128 d, uint64_t *quo, S1U128 *rem) {
 S1U128 r{0,0}, q{0,0};
 if(d.hi==0 && d.lo==0) { *quo=0; *rem=n; return; }
 for(int i=0;i<128;i++) {
  r=s1u_shl1(r); if(n.hi&0x8000000000000000ull) r.lo|=1; n=s1u_shl1(n); q=s1u_shl1(q);
  if(s1u_ge(r,d)) { r=s1u_sub(r,d); q.lo|=1; }
 }
 *quo = q.hi ? ~0ull : q.lo; *rem=r;
}
static inline int s1_fexp(float x, uint32_t *mant) {
 uint32_t u; memcpy(&u,&x,4); int e=(u>>23)&255;
 if(e==0 || e==255 || !(x>0.f)) { *mant=0; return -9999; }
 *mant=(u&0x7fffffu)|0x800000u; return e-127-23;
}
static inline S1U128 s1_shl24(uint32_t m, int sh) {
 if(sh<0 || sh>=128) return {~0ull,~0ull};
 if(sh>=64) return {(uint64_t)m<<(sh-64), 0};
 S1U128 r{0,(uint64_t)m<<sh}; if(sh>40) r.hi=(uint64_t)m>>(64-sh); return r;
}
static inline S1U128 s1_float_ticks(float x) {
 uint32_t m; int e=s1_fexp(x,&m); if(e<-900) return {0,0}; int sh=e+64;
 if(sh>=128) return {~0ull,~0ull}; if(sh<0) return {0,0}; return s1_shl24(m,sh);
}
static inline S1Clock s1_add_ratio(S1Clock c, float numer, float denom) {
 uint32_t mn, md; int en=s1_fexp(numer,&mn), ed=s1_fexp(denom,&md);
 if(en<-900 || ed<-900) { c.ok=0; return c; }
 int sh=en-ed+64; if(sh<0) return c; if(sh>=104) { c.ok=0; return c; }
 S1U128 num=s1_shl24(mn,sh);
 if(md==0) { c.ok=0; return c; }
 uint64_t rem=num.hi%md, qhi=num.hi/md;
 uint64_t mid=(rem<<32)|(num.lo>>32); uint64_t qmid=mid/md; rem=mid%md;
 uint64_t low=(rem<<32)|(num.lo&0xffffffffu); uint64_t qlo=low/md; rem=low%md;
 if((rem<<1)>md || ((rem<<1)==md && (qlo&1))) { if(++qlo==0x100000000ull) { qlo=0; qmid++; } }
 if(qmid>0xffffffffull) { c.ok=0; return c; }
 uint64_t frac=c.frac+(qmid<<32|qlo); uint64_t carry=frac<c.frac;
 if(c.whole>~0ull-qhi-carry) { c.ok=0; return c; }
 c.frac=frac; c.whole+=qhi+carry; return c;
}
#endif
// The original/target note ABI is signed32; reject another int ABI at compile time.
static_assert(sizeof(int)==4,"original note ABI requires signed32 int");
namespace s1source_sequence {
struct SerializedSequence {
#if defined(S1_NO_DOUBLE)
 float sample_rate=0;
 uint64_t beat_whole=0, beat_frac=0;
 void set_sample_rate(float x){sample_rate=x;}
 void set_clock(uint64_t whole, uint64_t frac){beat_whole=whole; beat_frac=frac;}
#else
 double sample_rate=0,beat_time=0;
 void set_sample_rate(double x){sample_rate=x;} // cold, serialized with process
#endif
 int previous_held=0,step=0,npo=12;
 int last_notes[128]{};unsigned last_count=0,initial_zero_count=0;
 void set_notes_per_octave(int x){npo=x;} // owner/cold only, never concurrent with process
 int get_beat_count()const{return step;} // owner snapshot, not a concurrent UI load
 int notes_per_octave()const{return npo;}
 void init(){
  previous_held=0;
  // Source resize(1024), not reserve: preserve existing pending real notes,
  // then append zeros. Repeated init preserves beat_time and step.
  initial_zero_count=1024-last_count;
 }
 void reset(bool notes){if(notes)previous_held=0;last_count=initial_zero_count=0;}
 static bool in(float x,float a,float b){return x>=a&&x<=b;}
 static bool parameters_valid(const float*p,size_t n){
  if(!p||n!=150||!in(p[arpIsOn],0,1)||!in(p[arpIsSequencer],0,1)||
     !in(p[arpRate],1,200)||!in(p[arpSeqTempoMultiplier],1.f/64.f/1.5f,8)||
     !in(p[arpDirection],0,2)||!in(p[arpOctave],0,3)||!in(p[arpInterval],0,12)||!in(p[arpTotalSteps],1,16))return false;
  for(unsigned i=0;i<16;i++)if(!in(p[sequencerPattern00+i],-12,12)||
     !in(p[sequencerOctBoost00+i],0,1)||!in(p[sequencerNoteOn00+i],0,1))return false;
  return true;
 }
 template<class Sink>void off_pending(Sink &sink){
  for(unsigned i=0;i<last_count;i++)sink.off(last_notes[i]);
  for(unsigned i=0;i<initial_zero_count;i++)sink.off(0);
  last_count=initial_zero_count=0;
 }
 static bool tuning_valid(const float*p,size_t n,int candidate_npo){
  if(!parameters_valid(p,n))return false;
  int np=candidate_npo;if(np<=0)np=12;
  const float npof=static_cast<float>(np)/12.f;
  // Reject only arithmetic domains where the original's int conversions or
  // additions would be undefined. Ordinary/imported tunings are not clamped.
  const float interval_float=p[arpInterval]*npof;
  if(!(interval_float>=0.f&&interval_float<2147483648.f))return false;
  const int interval=static_cast<int>(interval_float),octaves=static_cast<int>(p[arpOctave])+1;
  if(static_cast<int64_t>(interval)*(octaves-1)+127>INT32_MAX)return false;
  for(unsigned i=0;i<16;i++){
   const float q=p[sequencerPattern00+i]*npof;
   if(!(q>=-2147483648.f&&q<2147483648.f))return false;
   const int note=static_cast<int>(q),boost=static_cast<int>(p[sequencerOctBoost00+i]);
   const int64_t shifted=note<0?static_cast<int64_t>(note)-static_cast<int64_t>(boost)*np:static_cast<int64_t>(note)+static_cast<int64_t>(boost)*np;
   if(shifted<INT32_MIN||shifted>INT32_MAX-127)return false;
  }
  return true;
 }
 template<class View>bool can_process(const float*p,size_t n,const View&held)const{
  if(!tuning_valid(p,n,notes_per_octave())||!__builtin_isfinite(sample_rate)||sample_rate<=0||
     held.count>128||(held.count&&!held.values)||last_count>128||initial_zero_count>1024-last_count)return false;
  for(unsigned i=0;i<held.count;i++)if(held.values[i].noteNumber<0||held.values[i].noteNumber>=128||
     held.values[i].velocity<0||held.values[i].velocity>127)return false;
  const bool on=p[arpIsOn]==1.f,first_any=previous_held==0&&held.count>0,first_none=held.count==0&&previous_held>0;
#if defined(S1_NO_DOUBLE)
  S1Clock base{(on&&(first_none||first_any))?0:beat_whole,(on&&(first_none||first_any))?0:beat_frac,1};
  if(on||last_count||initial_zero_count){
   S1Clock next=s1_add_ratio(base,p[arpRate]/60.f,sample_rate);
   if(!next.ok)return false;
   if(on&&held.count){
    uint64_t idx; S1U128 rem; s1u_divmod({next.whole,next.frac},s1_float_ticks(p[arpSeqTempoMultiplier]),&idx,&rem);
    if(idx>=2147483648ull)return false;
   }
  }
#else
  const double base=on&&(first_none||first_any)?0:beat_time;
  if(!__builtin_isfinite(base))return false;
  if(on||last_count||initial_zero_count){
   const auto next=base+(p[arpRate]/60.f)/sample_rate;
   if(!__builtin_isfinite(next)||(on&&held.count&&next/p[arpSeqTempoMultiplier]>=2147483648.0))return false;
  }
#endif
  return true;
 }
 template<class View,class Sink>bool process(const float*p,size_t n,const View &held,Sink &sink){
  // All controls/held views and owner calls are serialized; callbacks do not
  // mutate inputs/owner, recurse, or exceed the sink's output/event capacity.
  if(!can_process(p,n,held))return false;
  const bool on=p[arpIsOn]==1.f,first_any=previous_held==0&&held.count>0,first_none=held.count==0&&previous_held>0;
  const int octaves=static_cast<int>(p[arpOctave])+1;
  if(on&&(first_none||first_any)){
#if defined(S1_NO_DOUBLE)
   beat_whole=beat_frac=0;
#else
   beat_time=0;
#endif
   step=0;
   off_pending(sink);sink.beat();
  }
  if(on||last_count||initial_zero_count){
#if defined(S1_NO_DOUBLE)
   S1Clock cur{beat_whole,beat_frac,1};
   S1U128 mult=s1_float_ticks(p[arpSeqTempoMultiplier]);
   uint64_t ignore; S1U128 r0,r1; s1u_divmod({cur.whole,cur.frac},mult,&ignore,&r0);
   S1Clock next=s1_add_ratio(cur,p[arpRate]/60.f,sample_rate);
   if(!next.ok)return false;
   beat_whole=next.whole; beat_frac=next.frac;
   uint64_t beat_index; s1u_divmod({next.whole,next.frac},mult,&beat_index,&r1);
   const int crossed=s1u_lt(r1,r0);
#else
   const auto next=beat_time+(p[arpRate]/60.f)/sample_rate;
   const double r0=fmod(beat_time,p[arpSeqTempoMultiplier]);beat_time=next;
   const double r1=fmod(beat_time,p[arpSeqTempoMultiplier]);
   const int crossed=r1<r0;
   const int beat_index_double=static_cast<int>(beat_time/p[arpSeqTempoMultiplier]);
#endif
   if(first_any||crossed){
    off_pending(sink);
    if(on&&held.count){
     if(notes_per_octave()<=0)set_notes_per_octave(12);
     const int notes=notes_per_octave();
     const float factor=static_cast<float>(notes)/12.f;
     const bool sequencer=p[arpIsSequencer]==1.f;
     unsigned length;
     if(sequencer)length=static_cast<unsigned>(p[arpTotalSteps]>16?16:static_cast<int>(p[arpTotalSteps]));
     else {
      const unsigned m=static_cast<unsigned>(held.count)*octaves;
      length=static_cast<int>(p[arpDirection])==1&&m>1?2*m-2:m;
     }
     if(length){
#if defined(S1_NO_DOUBLE)
      const int beat=beat_index>0x7fffffffull?0x7fffffff:(int)beat_index;
#else
      const int beat=beat_index_double;
#endif
      step=beat;const unsigned position=static_cast<unsigned>(beat)%length;sink.beat();
      if(sequencer){
       const int enabled=static_cast<int>(p[sequencerNoteOn00+position]);
       const int boost=static_cast<int>(p[sequencerOctBoost00+position]);
       const int note=static_cast<int>(p[sequencerPattern00+position]*factor);
       const int offset=note<0?note-boost*notes:note+boost*notes;
       if(enabled==1)for(unsigned i=0;i<held.count;i++){
        const int pitch=held.values[i].noteNumber+offset;
        if(pitch>=0&&pitch<128){sink.on(pitch,held.values[i].velocity);last_notes[last_count++]=pitch;}
       }
      }else{
       // Source first reverses held AEArray order; UpDown omits BOTH tails.
       const unsigned m=static_cast<unsigned>(held.count)*octaves;
       const int mode=static_cast<int>(p[arpDirection]);
       bool descending=mode==2||(mode==1&&position>=m);
       unsigned k=position;if(mode==1&&position>=m)k=position-m+1;
       const unsigned index=descending?k%held.count:held.count-1-k%held.count;
       const int octave=descending?octaves-1-static_cast<int>(k/held.count):static_cast<int>(k/held.count);
       const int pitch=held.values[index].noteNumber+octave*static_cast<int>(p[arpInterval]*factor);
       if(pitch>=0&&pitch<128){sink.on(pitch,held.values[index].velocity);last_notes[last_count++]=pitch;}
      }
     }
    }
   }
  }
  previous_held=static_cast<int>(held.count);return true;
 }
};
}
