from pathlib import Path
import sys
src,out,which=sys.argv[1:]
assert which in ('up','both')
s=Path(src).read_text()
s=s.replace('#include <stdint.h>','#include <stdint.h>\n#include <string.h>')
a=s.index('typedef struct')
b=s.index('static float clock_2x',a)
ap=s[a:b]
packed_down='Q24 down[2][2];' if which=='both' else 'float down[2][2];'
ap=ap[ap.index('static inline float ap_pair'):] if which=='up' else ''
s=s[:a]+f'''/* Binary32 exponent and sign preserved; round away 8 mantissa bits. */
typedef struct {{uint8_t bytes[3];}} Q24;
typedef struct {{Q24 up[2][2]; {packed_down} float pole[4],condition;}} OTA;
_Static_assert(sizeof(OTA)=={44 if which=='both' else 48}, "q24 history layout");
static inline float unpack24(const Q24 *s) {{
    uint32_t u=((uint32_t)s->bytes[0]<<8)|((uint32_t)s->bytes[1]<<16)|((uint32_t)s->bytes[2]<<24);
    float v;
    memcpy(&v,&u,4);
    return v;
}}
static inline void pack24(Q24 *s,float y) {{
    uint32_t u;
    memcpy(&u,&y,4);
    const uint32_t q=(u+127u+((u>>8)&1u))>>8;
    s->bytes[0]=(uint8_t)q;
    s->bytes[1]=(uint8_t)(q>>8);
    s->bytes[2]=(uint8_t)(q>>16);
}}
static inline float ap_q24(float x,Q24 z[2],float a,float b) {{
    float mid=split_mul_add(a,x,unpack24(&z[0]));
    pack24(&z[0],x-a*mid);
    float y=split_mul_add(b,mid,unpack24(&z[1]));
    pack24(&z[1],mid-b*y);
    return y;
}}
'''+ap+s[b:]
for history in ['up']+(['down'] if which=='both' else []):
 for i in (0,1):
  start=s.index(f's->{history}[{i}]',s.index('static inline float physical_tick'))
  begin=s.rindex('ap_pair(',0,start)
  s=s[:begin]+'ap_q24('+s[begin+len('ap_pair('):]
Path(out).write_text(s)
