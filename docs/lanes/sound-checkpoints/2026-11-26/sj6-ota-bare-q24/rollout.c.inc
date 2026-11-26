#ifndef FN
#error "measurement function symbol required"
#endif
unsigned FN(const float *p,const float *c,unsigned n){
 OTA s={0};uint32_t h=2166136261u,r=0x71c5adf3;
 for(unsigned i=0;i<n;i++){
  r^=r<<13;r^=r>>17;r^=r<<5;
  float x=(float)((int32_t)(r&65535u)-32768)*(1.f/524288.f);
  union {float f;uint32_t u;} y={.f=physical_tick(&s,x,p,c)};
  h=(h^y.u)*16777619u;
 }
 return h;
}
