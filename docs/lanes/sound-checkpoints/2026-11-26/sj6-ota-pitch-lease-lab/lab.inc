/* Never-install diagnostic: explicit caller-owned references; no global lease. */
typedef struct { float up[2][2],down[2][2],condition; } MainAP;
typedef struct { MainAP v[6]; float pole0_v4; } GapA;
typedef struct { float poles01[4][2]; } GapB;
typedef struct { float pole1_v4,poles01_v5[2]; } GapC;
_Static_assert(sizeof(GapA)==220 && sizeof(GapB)==32 && sizeof(GapC)==12,"typed gap preimages");
typedef struct { GapA *a; GapB *b; GapC *c; } ColdLease;
typedef struct {
  float board_hz,rate,peak_limit,gain;
  const float *original_params,*coeff;
} ColdProfile;
typedef struct {
  float physical_hz,rate,feedback,main,main_gain,wave;
  unsigned source_family,ota,enabled,pitch_depth,pitch_routed,no_noise,no_legacy_sub;
} FrameFingerprint;
/* Return 1 eligible, 0 unconfigured/lab off, -1 invalidate WHOLE test.
 * Caller may never continue playback on error, nor borrow while pitch arms. */
static int match(const ColdProfile *p,const FrameFingerprint *f) {
 if(!f->enabled)return 0;
 if(!p || !p->original_params || !p->coeff ||
    !(f->rate==p->rate && f->physical_hz==p->board_hz) ||
    !f->ota || f->source_family!=DNBX_OSC_SUPERJ6 || f->wave!=2.f/3.f ||
    f->feedback!=0.f || f->pitch_depth || f->pitch_routed ||
    !f->no_noise || !f->no_legacy_sub ||
    !(f->main_gain==p->gain && f->main>=-p->peak_limit && f->main<=p->peak_limit))return -1;
 return 1;
}
static void history_refs(ColdLease l,unsigned i,float **p0,float **p1) {
 if(i<4){*p0=&l.b->poles01[i][0];*p1=&l.b->poles01[i][1];}
 else if(i==4){*p0=&l.a->pole0_v4;*p1=&l.c->pole1_v4;}
 else {*p0=&l.c->poles01_v5[0];*p1=&l.c->poles01_v5[1];}
}
/* Invoke only at same actual allocator-owned whole voice cold memset, not
 * same-note/release tail. Caller validates allocation/control ownership. */
static void reset_cold(ColdLease l,dnbx_voice *v,unsigned i){
 float *p0,*p1;history_refs(l,i,&p0,&p1);
 l.a->v[i]=(MainAP){0};
 *p0=*p1=0.f;
 v->ota_main_lease.pole2=v->ota_main_lease.pole3=0.f;
}
static int main_plus_untouched_sub(ColdLease l,dnbx_voice *v,unsigned i,
 const ColdProfile *p,const FrameFingerprint *f,float stock_sub_filtered,float *result){
 const int sel=match(p,f);
 if(sel<=0)return sel;
 if(i>=6)return -1;
 float *p0,*p1;history_refs(l,i,&p0,&p1);
 MainAP *a=&l.a->v[i];
 /* Named Float32 copy only, not a cast across different owners or fields.
  * This source-only call-site validator makes NO efficiency claim. */
 OTA s={{ {a->up[0][0],a->up[0][1]}, {a->up[1][0],a->up[1][1]} },
        { {a->down[0][0],a->down[0][1]}, {a->down[1][0],a->down[1][1]} },
        {*p0,*p1,v->ota_main_lease.pole2,v->ota_main_lease.pole3},
        a->condition};
 const float y=physical_tick(&s,f->main,p->original_params,p->coeff);
 for(unsigned j=0;j<2;j++)for(unsigned k=0;k<2;k++){ a->up[j][k]=s.up[j][k];a->down[j][k]=s.down[j][k];}
 *p0=s.pole[0];*p1=s.pole[1];v->ota_main_lease.pole2=s.pole[2];v->ota_main_lease.pole3=s.pole[3];
 a->condition=s.condition;
 *result=(y*f->main_gain)+stock_sub_filtered;
 return 1;
}
/* Bind to one cold diagnostic run before first whole-voice allocation.
 * A bad sample poisons the diagnostic; restoring control values cannot
 * resume later on a half-advanced history. Stock is never selected here. */
typedef struct {const ColdProfile *profile; unsigned enabled,failed;} ColdRun;
static int guarded_main_sample(ColdRun *run,ColdLease l,dnbx_voice *v,unsigned i,
 const FrameFingerprint *f,float stock_sub,float *out){
 if(!run->enabled)return 0;
 if(run->failed)return -1;
 const int ok=main_plus_untouched_sub(l,v,i,run->profile,f,stock_sub,out);
 if(ok!=1){run->failed=1;return -1;}
 return 1;
}
