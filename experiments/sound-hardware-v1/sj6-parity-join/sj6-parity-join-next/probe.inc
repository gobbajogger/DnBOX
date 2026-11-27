#include <stdlib.h>
extern void *sj6_new(double);
extern void sj6_free(void *);
extern void sj6_params(void *,double,float,float,int);
extern void sj6_process(void *,size_t,const float *,const float *,float *,float *);

struct lab_audio_owner {
    original_runtime_part p;
    original_runtime_workspace w;
    int16_t unison[2][SJ6_UNISON_HISTORY];
    void *delay;
};
void *lab_open(float delay,float feedback,float dry,unsigned stages)
{
    struct lab_audio_owner *q=calloc(1,sizeof *q);
    if(!q)return NULL;
    original_runtime_part *p=&q->p;
    juno6_t *s=&p->dsp.juno.synth;
    p->kind=ORIGINAL_SOUND_JUNO;
    p->status=ORIGINAL_RUNTIME_OK;
    s->core.sample_rate=44100.f;
    s->master_gain=.83f;
    s->hpf=.32f;
    p->dsp.juno.chorus.sample_rate=44100.f;
    p->dsp.juno.chorus.factory_mode=stages&2u?JUNO6_CHORUS_II:JUNO6_CHORUS_OFF;
    if(stages&2u) {
        uint8_t code[3]={0};
        juno6_classic_native_config(&p->dsp.juno.chorus,code);
    }
    if(stages&1u){
        const uint8_t code[]={1,123,96,1};
        juno6_unison_config(s,code,q->unison);
    }
    if(stages&4u){
        q->delay=sj6_new(44100.);
        if(!q->delay){free(q);return NULL;}
        s->delay24.lab_fullrate=q->delay;
        s->delay24.lab_frames=delay;
        s->delay24.lab_feedback=feedback;
        s->delay24.lab_dry=dry;
    }
    if(stages&8u){
        p->lab_room=compose_room_create();
        if(!p->lab_room){sj6_free(q->delay);free(q);return NULL;}
        compose_room_feedback(p->lab_room,.84f);
        if(stages&16u)original_runtime_sj6_rage(p,8u);
    }
    return q;
}
void lab_close(void *raw) {
    struct lab_audio_owner *q=raw;
    sj6_free(q->delay);compose_room_destroy(q->p.lab_room);free(q);
}

float lab_unison_head(void *raw,unsigned i) {
    struct lab_audio_owner *q=raw;
    return q->p.dsp.juno.synth.unison.head[i];
}
int lab_graph(void *raw,const sj6_source_block *b,float *l,float *r,unsigned n,unsigned direct,const RoomBlock *room)
{
    struct lab_audio_owner *q=raw;
    original_runtime_part *p=&q->p;
    juno6_t *s=&p->dsp.juno.synth;
    if(room&&!compose_room_admits(p->lab_room,room,l,r,n))return 0;
    if(!direct) {
       p->lab_room_block=room;

       p->sj6_source_pending=b;
       const int result=render_backend(p,&q->w,l,r,n);
       p->lab_room_block=NULL;
       return result;
    }
    for (unsigned i=0;i<n;i++){
      float a=b->left[0][i],z=b->right[0][i];
      for (unsigned k=1;k<4;k++){a+=b->left[k][i];z+=b->right[k][i];}
      l[i]=a*b->trim;r[i]=z*b->trim;
    }
    juno6_unison_process(s,l,r,n);
    juno6_sj6_hpf_bus(s,l,r,n);
    if(p->dsp.juno.chorus.classic_native.typed)
      juno6_classic_native_process(&p->dsp.juno.chorus,l,r,n);
    if(q->delay){
      /* Independent uninterrupted inner call within caller's callback; sample
       * partition need not match live path for settled controls. */
      sj6_params(q->delay,(double)s->delay24.lab_frames/44100.,
                 s->delay24.lab_feedback,s->delay24.lab_dry,0);
      sj6_process(q->delay,n,l,r,l,r);
    }
    if(room){
        /* Test explicit independent sum: Room sees only the prevalidated
         * wet input, therefore no combined-output compression. */
        float wl[256]={0},wr[256]={0};
        sj6_limiter_process(&p->dsp.juno.limiter,l,r,n);
        compose_room_tick(p->lab_room,room,wl,wr,n);
        for(unsigned i=0;i<n;i++){l[i]+=wl[i];r[i]+=wr[i];}
    }
    for (unsigned i=0;i<n;i++){l[i]*=s->master_gain;r[i]*=s->master_gain;}
    return 1;
}
