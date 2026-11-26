from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text().replace('#include <stdint.h>','#include <stdint.h>\n#include <string.h>')
s=s.replace('typedef struct {float up[2][2],down[2][2],pole[4],condition;} OTA;','typedef struct {uint32_t upbits[3],downbits[3];float pole[4],condition;} OTA;')
s=s.replace('sizeof(OTA)==52','sizeof(OTA)==44')
where=s.index('static inline float ap_pair')
code='''/* Each group is four Q24 histories in three aligned words (no crossing). */
static inline float f24(uint32_t u){
    float y;
    u <<= 8;
    __builtin_memcpy(&y,&u,4);
    return y;
}
static inline uint32_t q24(float y){
    uint32_t u;
    __builtin_memcpy(&u,&y,4);
    return (u+127u+((u>>8)&1u))>>8;
}
static __attribute__((noinline)) void unpack4(const uint32_t q[3],float v[2][2]){
    const uint32_t a=q[0],b=q[1],c=q[2];
    v[0][0]=f24(a);
    v[0][1]=f24((a>>24)|(b<<8));
    v[1][0]=f24((b>>16)|(c<<16));
    v[1][1]=f24(c>>8);
}
static __attribute__((noinline)) void pack4(uint32_t q[3],const float v[2][2]){
    const uint32_t a=q24(v[0][0]),b=q24(v[0][1]),c=q24(v[1][0]),d=q24(v[1][1]);
    q[0]=a|(b<<24);
    q[1]=(b>>8)|(c<<16);
    q[2]=(c>>16)|(d<<8);
}
'''
s=s[:where]+code+s[where:]
start=s.index('static inline float physical_tick')
a=s.index('    float a=ap_pair',start); b=s.index('    a=clock_2x',a)
s=s[:a]+'''    float history[2][2];
    unpack4(s->upbits,history);
    float a=ap_pair(x,history[0],coeff[0],coeff[1]);
    float b=ap_pair(x,history[1],coeff[2],coeff[3]);
    pack4(s->upbits,history);
'''+s[b:]
a=s.index('    b=ap_pair',start);b=s.index('    return 40.f',a)
s=s[:a]+'''    unpack4(s->downbits,history);
    b=ap_pair(b,history[0],coeff[0],coeff[1]);
    a=ap_pair(a,history[1],coeff[2],coeff[3]);
    pack4(s->downbits,history);
'''+s[b:]
Path(sys.argv[2]).write_text(s)
