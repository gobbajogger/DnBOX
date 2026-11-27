/* Host reference only. Matched 3072 tapped settings and 12 reached live edits;
 * native math, per-source persistent histories and full sound are not qualified. */
#ifndef LAB_SJ6_PHYSICAL_K_H
#define LAB_SJ6_PHYSICAL_K_H
#include <math.h>
/* Input r is the physical Voice constructor's scalar, NOT the panel word. */
static inline float sj6_ota_atten_to_r(float attenuation) {
 return (-20.f*log10f(fmaxf(attenuation,.0001f)))/0x1.2fd70ap+4f;
}
static inline float sj6_ota_r_to_k(float r,float hz,float fs) {
 r=fmaxf(0.f,fminf(r,1.f));
 float v,k;
 if(r<=.5f) {
  v=r/.9f;
  k=(v*v)*fmaf(v,-0x1.bae148p+2f,0x1.5d70a4p+3f);
 } else if(r<.75f){
  v=(r-.5f)*4.f;
  k=fmaf(v,fmaf(v,fmaf(v,0x1.90faa0p-1f,-0x1.6852a0p+0f),0x1.972d50p+0f),0x1.178734p+1f);
 } else if(r<.9f){
  v=(r-.75f)/.15f;
  k=fmaf(v,fmaf(v,fmaf(v,-0x1.b0a3d2p-1f,0x1.051eb4p+0f),0x1.59999ap-1f),0x1.933334p+1f);
 } else k=fmaf(r-.9f,0x1.333334p+0f,4.f);
 hz=fmaxf(3.f,fminf(hz,fminf(fs*.9f,30000.f)));
 const float t=fmaxf(0.f,fminf(1.f,((hz/fs)+(-0x1.ccccccp-2f))/.05f));
 const float excess=fmaxf(k-0x1.f33334p+1f,0.f)*t;
 return fmaf(-(t*excess),fmaf(t,-2.f,3.f),k);
}
/* The pinned Apple import is exp10. Portable bench uses pow(10,x);
 * measured equivalent for supported physical words, not a target libm claim. */
static inline float sj6_ota_physical_db_to_atten(float db) {
 return (float)pow(10.,(double)db*(-.05));
}
static inline float sj6_ota_panel_to_k(float u,float hz,float fs) {
 const float db=(float)(18.99*(double)u);
 return sj6_ota_r_to_k(sj6_ota_atten_to_r(sj6_ota_physical_db_to_atten(db)),hz,fs);
}
#endif
