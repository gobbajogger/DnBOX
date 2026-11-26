#include "morph-native.inc"
volatile unsigned dummy;
uint32_t prepared(float f){return prep_morph(f);}
float tick(uint32_t *p,uint32_t plan,float x,int b){return tick_morph(p,plan,x,b);}
void pair(uint32_t out[2],unsigned row, unsigned phase){
 sample_pair v=stored_pair(row,phase);
 union{float f;uint32_t u;} a={.f=v.a},b={.f=v.b};
 out[0]=a.u;out[1]=b.u;
}
