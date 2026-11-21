#ifndef DNBOX_TEST_QUAD_GUARD_HPP
#define DNBOX_TEST_QUAD_GUARD_HPP
// Include after the retained compact test fixture. Preserve its oracle-window,
// actual word-read counters and deliberate finite-wrong-word negative control.
static unsigned long long compact_quad_calls=0;
static bool compact_quad_wrong_lsb=false;
extern "C" int __real_s1c_read_quad(const s1c_binding*,unsigned,unsigned,unsigned,unsigned,uint32_t*);
extern "C" int __wrap_s1c_read_quad(const s1c_binding*b,unsigned r0,unsigned r1,unsigned p0,unsigned p1,uint32_t*out){
 CHECK(!compact_oracle_window);CHECK(r0<52&&r1<52&&p0<4097&&p1<4097);
 ++compact_quad_calls;compact_word_reads+=4;compact_rows[r0]+=2;compact_rows[r1]+=2;
 const int result=__real_s1c_read_quad(b,r0,r1,p0,p1,out);
 if(result&&(compact_wrong_lsb||compact_quad_wrong_lsb))for(unsigned i=0;i<4;i++)out[i]^=1u;
 return result;
}
#endif
