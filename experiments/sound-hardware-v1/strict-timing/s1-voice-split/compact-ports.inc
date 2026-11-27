/* Include inside exact s1c.c TU after stock voice_run_kr. No shared writes except
 * each helper-owned v[] element and prefix output, no global diagnostics. */
typedef struct {
 float c[46]; s1u_lfo lfo[2];
 const float *t; float sr,c1,c2,lfo_c2; uint32_t rnd;
} s1_voice_view;
#if UINTPTR_MAX == 0xffffffffu
_Static_assert(sizeof(s1_voice_view)==240,"PI32 control view");
#endif
static float split_lfo01(const s1_voice_view *s,float sel){
 int k=(int)sel;
 return k==1?s->lfo[0].o01:k==2?s->lfo[1].o01:k==3?.5f*(s->lfo[0].o01+s->lfo[1].o01):0.f;
}
static float split_lfo10(const s1_voice_view *s,float sel){
 int k=(int)sel;
 return k==1?s->lfo[0].o10:k==2?s->lfo[1].o10:k==3?.5f*(s->lfo[0].o10+s->lfo[1].o10):1.f;
}
typedef struct { float target[46]; uint8_t slot[46], count; } s1_voice_port_span;
typedef struct {const float *target; const uint8_t *slot; unsigned count;} s1_voice_port_view;
#if UINTPTR_MAX == 0xffffffffu
_Static_assert(sizeof(s1_voice_port_view)==12,"borrowed PI32 port descriptor");
#endif
/* At the original render-span preamble; zero wet mask for untiled owner,
 * or omit independently consumed wet-only lanes from this front view. */
void s1_voice_port_plan(s1_voice_port_span *p,const s1_voice_view *s,uint32_t wetlo,uint32_t wethi){
 unsigned k=0;
 for(unsigned j=0;j<sizeof s1c_smid;j++){
   if((j<32u?wetlo>>j:wethi>>(j-32u))&1u)continue;
   const float t=s->c1*s->t[s1c_smid[j]];
   const s1c_fb cur={.f=s->c[j]},nxt={.f=t+s->c2*cur.f};
   if(nxt.u!=cur.u){p->target[k]=t;p->slot[k++]=(uint8_t)j;}
 }
 p->count=(uint8_t)k;
}
void s1_voice_capture(s1_voice_view *c,const s1c *owner){
 __builtin_memcpy(c->c,owner->c,sizeof c->c);
 __builtin_memcpy(c->lfo,owner->lfo,sizeof c->lfo);
 c->t=owner->t;c->sr=owner->sr;c->c2=owner->port_c2;c->c1=1.f-c->c2;
 c->lfo_c2=owner->lfo_c2;c->rnd=owner->rnd;
}
static float __attribute__((noinline)) voice_run_compact(s1_voice_view *s, s1c_voice *v,uint32_t draw)
{
    int ft = (int)P(FTYPE);
    v->amp = env_step(&v->ea, v->gate, P(AA), clampf(P(AD) * split_lfo10(s, P(LDEC)), 0.005f, 2.0f), P(AS), P(AR), s->sr);
    v->fenv = env_step(&v->ef, v->gate, P(FA), P(FD), P(FS), P(FR), s->sr);
    if (--v->kc <= 0) {
        v->kc = S1C_KR;
        float base = v->freq, half = 0.5f * s->sr;
        float plfo = 1.0f + split_lfo01(s, P(LPITCH)) * 0.0594630944f, k = P(DETM) * plfo;
        const float f1 = clampf(base * exp2f((int)P(SEMI1) / 12.0f) * k, 0.0f, half);
        float det = P(DET2) * (v->freq / 261.6255653f);
        if ((int)P(LDET)) det *= split_lfo01(s, P(LDET));
        const float f2 = clampf(base * exp2f((int)P(SEMI2) / 12.0f) * k + det, 0.0f, half);
        const float fs = clampf(base * k / (2.0f * (1.0f + P(SUBOCT))), 0.0f, half);
        v->kff = clampf(base * k, 0.0f, half);
        /* The frequencies and the sample rate cannot change until the next
         * control tick. Save the original rounded divisions and phase words
         * in the existing cache, not additional voice state. */
        v->kd1 = f1 / s->sr; v->kd2 = f2 / s->sr;
        v->ki1 = ph_inc(v->kd1); v->ki2 = ph_inc(v->kd2);
        v->kis = ph_inc(fs / s->sr); v->kif = ph_inc(v->kff / s->sr);
        v->kidx = clampf(P(FMA) * split_lfo10(s, P(LFM)), 0.0f, 15.0f);
        v->kbal = clampf(P(BAL) + split_lfo01(s, P(LMIX)), 0.0f, 1.0f);
        const float kres = clampf(P(RES) * split_lfo10(s, P(LRES)), 0.0f, 0.98f);
        float pitch = log2f(f1 > 0.0f ? f1 : 261.0f), kt = 1.0f - clampf((pitch - 6.0f) / 5.0f, 0.0f, 1.0f);
        float kf = 1.0f - P(KTRK);
        v->kkey = (1.0f - kf) * kt * kt + kf;
        const float kcut = P(CUT) * split_lfo10(s, P(LCUT)), kem = P(FENV) * split_lfo10(s, P(LFENV));
        float cut = clampf(kcut - kcut * kem * (1.0f - v->fenv), 64.0f, 22050.0f);
        if (ft == 0) {
            float x = cut * (2.0f / s->sr); if (x > 1.0f) x = 1.0f;
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
            v->ka1 = kres; /* unused by LP; retain original KR resonance */
#endif
            v->kA = ladder_a(x);
            v->kfb = kres * (3.98720139f + x * (3.68178143f - x * 3.93633599f));
        } else if (ft == 1) {
            float bw = 0.0625f * s->sr * (exp2f(clampf(1.0f - kres, 0.0f, 1.0f)) - 1.0f);
            float cc = 1.0f / tanf(3.14159265f * bw / s->sr), d = 2.0f * cosf(TWO_PI * cut / s->sr), a1 = 1.0f / (1.0f + cc);
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
            v->kA = cc; /* unused by BP; retain original KR bandwidth */
#endif
            v->ka1 = a1; v->ka4 = -cc * d * a1; v->ka5 = (cc - 1.0f) * a1;
        } else {
            float cc = tanf(3.14159265f * cut / s->sr), a1 = 1.0f / (1.0f + 1.41421356f * cc + cc * cc);
            v->ka1 = a1; v->ka4 = 2.0f * (cc * cc - 1.0f) * a1; v->ka5 = (1.0f - 1.41421356f * cc + cc * cc) * a1;
        }
    }
    int bl = P(BLE) > 0.0f;
    float o1 = morph(&v->ph[0], v->kd1, v->ki1, P(I1), bl) * v->vamp * P(VOL1);
    float o2 = morph(&v->ph[1], v->kd2, v->ki2, P(I2), bl) * v->vamp * P(VOL2);
    float y = (1.0f - v->kbal) * o1 + v->kbal * o2;
    float sub = sine_t(v->ph[2]);
    v->ph[2] += v->kis;
    y += P(SUBSQ) != 0.0f ? (sub > 0.0f ? P(SUBV) : -P(SUBV)) : sub * v->vamp * P(SUBV) * 3.0f;
    if (P(FMV) != 0.0f) {
        float mod = sine_t(v->mph) * v->kff * v->kidx;
        v->mph += v->kif;
        y += sine_t(v->cph) * v->vamp * P(FMV);
        v->cph += ph_inc((v->kff + mod) / s->sr);
    }
    if (P(NOISEV) != 0.0f) {

        y += ((float)(int32_t)draw * (1.0f / 2147483648.0f)) * v->vamp * P(NOISEV) * split_lfo10(s, P(LNOISE));
    }
    y *= v->amp * v->kkey;
    float fo;
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    const float route=P(LFENV);
    if(P(FENV)>0.f && ((route==1.f && P(L1A)>0.f) ||
                      (route==2.f && P(L2A)>0.f) ||
                      (route==3.f && (P(L1A)>0.f || P(L2A)>0.f)))) {
        const float base=P(CUT)*split_lfo10(s,P(LCUT)), amount=P(FENV)*split_lfo10(s,route);
        const float hz=clampf(base-base*amount*(1.f-v->fenv),64.f,22050.f);
        if(ft==0){
            float x=hz*(2.f/s->sr);if(x>1.f)x=1.f;
            const float a=ladder_a(x),fb=v->ka1*(3.98720139f+x*(3.68178143f-x*3.93633599f));
            fo=ladder(v->lp,y,a,fb);
        } else if(ft==2){
            const float cc=tanf(3.14159265f*hz/s->sr),a1=1.0f/(1.0f+1.41421356f*cc+cc*cc);
            const float a4=2.0f*(cc*cc-1.0f)*a1,a5=(1.0f-1.41421356f*cc+cc*cc)*a1;
            fo=hp2(v->bq,y,a1,a4,a5);
        } else {
            const float d=2.f*cosf(TWO_PI*hz/s->sr);
            const float a4=-v->kA*d*v->ka1;
            float t=y-a4*v->bq[0]-v->ka5*v->bq[1];
            fo=t*v->ka1-v->ka1*v->bq[1];
            v->bq[1]=v->bq[0];v->bq[0]=t;
        }
    } else
#endif
    if (ft == 0) fo = ladder(v->lp, y, v->kA, v->kfb);
    else if (ft == 2) fo = hp2(v->bq, y, v->ka1, v->ka4, v->ka5);
    else {
        float t = y - v->ka4 * v->bq[0] - v->ka5 * v->bq[1];
        fo = t * v->ka1 - v->ka1 * v->bq[1];
        v->bq[1] = v->bq[0]; v->bq[0] = t;
    }
    return y + P(FMIX) * (fo - y);
}

static inline uint32_t split_lcg(uint32_t x){return x*1664525u+1013904223u;}
/* cut is a caller-selected ascending partition boundary, stable for this tile.
 * First worker returns one prefix-sum word/frame, second returns ordered suffix
 * voices. This is not the combiner. Caller commits only home terminal controls. */
void s1_voice_tile(s1_voice_view *s,s1c_voice *voices,unsigned active,unsigned n,
                   unsigned cut,unsigned home,float *out,const s1_voice_port_view *p){
 for(unsigned f=0;f<n;f++){
   for(unsigned j=0;j<p->count;j++){
     const unsigned k=p->slot[j];s->c[k]=p->target[j]+s->c2*s->c[k];
   }
   s1u_lfo_tick(&s->lfo[0],P(L1R),(int)P(L1I),P(L1A),s->lfo_c2,s->sr);
   s1u_lfo_tick(&s->lfo[1],P(L2R),(int)P(L2I),P(L2A),s->lfo_c2,s->sr);
   const unsigned noise=P(NOISEV)!=0.f;
   float prefix=0.f;
   for(unsigned v=0;v<S1C_VOICES;v++)if(active&(1u<<v)){
      if(noise)s->rnd=split_lcg(s->rnd);
      if(!home && v<cut)prefix+=voice_run_compact(s,&voices[v],s->rnd);
      if(home && v>=cut)out[f*(S1C_VOICES-cut)+v-cut]=voice_run_compact(s,&voices[v],s->rnd);
   }
   if(!home)out[f]=prefix;
 }
}
