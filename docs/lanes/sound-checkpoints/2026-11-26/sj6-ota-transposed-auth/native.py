import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text().split('__attribute__((visibility("default")))',1)[0]
if 'float up[2][2],down[2][2]' in s:
 s=s.replace('float up[2][2],down[2][2],pole[4],condition;', 'float up[2][2],down[2][2],condition;').replace('sizeof(OTA)==52','sizeof(OTA)==36')
 s=s.replace('clock_2x(OTA *s,float x','clock_2x(OTA *s,float pole[4],float x').replace('s->pole','pole').replace('clock_2x(s,a','clock_2x(s,pole,a').replace('clock_2x(s,b','clock_2x(s,pole,b')
 s=s.replace('static inline float physical_tick(OTA *s, float x','static __attribute__((noinline)) float physical_tick(OTA *s,float pole[4],float x')
 s+='''typedef struct { OTA v[6]; float spare; } GapA;
_Static_assert(sizeof(GapA)==220 && _Alignof(GapA)==_Alignof(float), "typed gapA");
float admit(GapA *a,float pole[4],float x,const float *p,const float *c,unsigned mask,unsigned v,float stock){
 if(((mask>>v)&1u)==0)return stock;
 return physical_tick(&a->v[v],pole,x,p,c);
}'''
else:s=s.replace('static inline float physical_tick','float physical_tick')
Path(sys.argv[2]).write_text(s)
