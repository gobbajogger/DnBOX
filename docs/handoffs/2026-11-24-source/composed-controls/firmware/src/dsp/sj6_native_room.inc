/* SPDX-License-Identifier: GPL-3.0-only */
/* Independent compact stereo inserts. No shared sends, cross-owner state,
 * malloc, floating-point delay storage, or overlap with the Delay arena. */
#if X0X_SJ6_NATIVE_ROOM
#include "../dsp/fastmath.h"
float sj6_room_rate(unsigned word)
{
    float u=(float)word*(1.f/127.f);
    /* Original free rate, authenticated Build9 conductor ID22. */
    return 16.f*fm_powf(u,3.33f);
}
float sj6_room_rate_clocked(unsigned word,float bpm,int sync)
{
    if(!sync)return sj6_room_rate(word);
    static const float grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
    const unsigned index=(17u*(127u-word)+63u)/127u;
    return bpm>0.f ? (bpm*(1.f/240.f))*grid[index] : 0.f;
}
void original_runtime_sj6_room_clock(original_runtime_part *p,float bpm,int sync)
{
    if(!p || p->kind!=ORIGINAL_SOUND_JUNO || p->status!=ORIGINAL_RUNTIME_OK)return;
    sj6_room_t *s=&p->dsp.juno.room;
    if(s->clock_bpm==bpm && s->clock_sync==(unsigned)!!sync)return;
    s->clock_bpm=bpm;s->clock_sync=(uint8_t)!!sync;
    s->rate=sj6_room_rate_clocked(s->rate_code,bpm,sync)*(1.f/44100.f);
}
float sj6_room_locut(unsigned word)
{
    /* Independent wet HPF ID14, not the similarly scaled instrument HPF. */
    return 800.f*fm_powf((float)word*(1.f/127.f),2.42f);
}
static void sj6_room_reset(sj6_room_t *s)
{
    memset(s->ring,0,sizeof s->ring);
    memset(s->pos,0,sizeof s->pos);
    memset(s->lo,0,sizeof s->lo);
    memset(s->acc,0,sizeof s->acc);
    memset(s->wet,0,sizeof s->wet);
    memset(s->last,0,sizeof s->last);
    s->tick=0;
}
void original_runtime_sj6_room(original_runtime_part *p,const uint8_t word[SJ6_ROOM_COUNT])
{
    if(!p||p->kind!=ORIGINAL_SOUND_JUNO||p->status!=ORIGINAL_RUNTIME_OK)return;
    sj6_room_t *s=&p->dsp.juno.room;
    if(s->enable!=word[SJ6_ROOM_ENABLE]||s->type!=word[SJ6_ROOM_TYPE])sj6_room_reset(s);
    if(s->widen!=word[SJ6_ROOM_WIDEN]){
        memset(s->wide,0,sizeof s->wide);
        s->wide_pos=0;
    }
    s->armed=1;
    s->enable=word[SJ6_ROOM_ENABLE];s->type=word[SJ6_ROOM_TYPE];
    s->widen=word[SJ6_ROOM_WIDEN];
    s->mix=(float)word[SJ6_ROOM_MIX]*(1.f/127.f);
    s->fb=.4f+(.6f/127.f)*(float)word[SJ6_ROOM_SIZE];
    s->hp_coeff=1.f-fm_expf(-6.2831853f*sj6_room_locut(word[SJ6_ROOM_LOCUT])*
                              (SJ6_ROOM_STRIDE/44100.f));
    s->rate_code=word[SJ6_ROOM_RATE];
    s->rate=sj6_room_rate_clocked(s->rate_code,s->clock_bpm,s->clock_sync)*(1.f/44100.f);
    s->depth=(float)word[SJ6_ROOM_DEPTH]*(1.f/127.f);
}
static int16_t sj6_room_pcm(float x)
{
    if(x>1.f)x=1.f;
    if(x< -1.f)x=-1.f;
    return (int16_t)(32767.f*x);
}
static void sj6_room_tick(sj6_room_t *s)
{
    /* Two compact room responses: paired asymmetric combs and a four-line
     * diffusing network. Neither stores or invokes the app's original IR. */
    static const uint8_t len[2][4]={{47,59,73,89},{61,79,97,113}};
    const uint8_t *length=len[s->type];
    float x[4],in[2],feedback[4];
    for(unsigned c=0;c<2;++c){
        float y=s->acc[c]*(1.f/SJ6_ROOM_STRIDE);
        s->acc[c]=0.f;
        s->lo[c]+=s->hp_coeff*(y-s->lo[c]);
        in[c]=y-s->lo[c];
        s->last[c]=s->wet[c];
    }
    for(unsigned i=0;i<4;++i)x[i]=(1.f/32767.f)*s->ring[i][s->pos[i]];
    if(s->type){
        feedback[0]=.5f*(x[0]+x[1]+x[2]+x[3]);
        feedback[1]=.5f*(x[0]-x[1]+x[2]-x[3]);
        feedback[2]=.5f*(x[0]+x[1]-x[2]-x[3]);
        feedback[3]=.5f*(x[0]-x[1]-x[2]+x[3]);
        s->wet[0]=.55f*(x[0]+x[3]);
        s->wet[1]=.55f*(x[1]-x[2]);
    }else{
        feedback[0]=x[0];feedback[1]=x[1];feedback[2]=x[2];feedback[3]=x[3];
        s->wet[0]=.55f*(x[0]+x[2]);
        s->wet[1]=.55f*(x[1]+x[3]);
    }
    for(unsigned i=0;i<4;++i){
        s->ring[i][s->pos[i]]=sj6_room_pcm(.42f*in[i&1u]+s->fb*feedback[i]);
        if(++s->pos[i]>=length[i])s->pos[i]=0;
    }
}
static void sj6_room_process(sj6_room_t *s,float *left,float *right,unsigned frames)
{
    if(!s->armed||(!s->enable&&!s->widen&&s->depth==0.f))return;
    for(unsigned i=0;i<frames;++i){
        float l=left[i],r=right[i];
        if(s->enable){
            s->acc[0]+=l;s->acc[1]+=r;
            if(++s->tick==SJ6_ROOM_STRIDE){s->tick=0;sj6_room_tick(s);}
            if(s->mix>0.f){
                float t=(float)s->tick*(1.f/SJ6_ROOM_STRIDE);
                l+=(s->last[0]+t*(s->wet[0]-s->last[0])-l)*s->mix;
                r+=(s->last[1]+t*(s->wet[1]-s->last[1])-r)*s->mix;
            }
        }
        if(s->widen){
            /* The dry midpoint is unchanged; only the differential path is
             * widened. A short all-pass decorrelates even a centered input. */
            float m=.5f*(l+r);
            float z=s->wide[s->wide_pos]*(1.f/32767.f);
            float ap=z-.5f*m;
            s->wide[s->wide_pos]=sj6_room_pcm(m+.5f*ap);
            if(++s->wide_pos==SJ6_ROOM_WIDE)s->wide_pos=0;
            float side=.30f*(m-ap);
            l+=side;r-=side;
        }
        if(s->depth>0.f){
            float pan=fm_sinf(6.2831853f*s->pan_phase)*s->depth;
            /* Center exactly unity, continuously shift opposing stereo gains. */
            l*=1.f-pan;r*=1.f+pan;
            s->pan_phase+=s->rate;
            if(s->pan_phase>=1.f)s->pan_phase-=1.f;
        }
        left[i]=l;right[i]=r;
    }
}
#endif
