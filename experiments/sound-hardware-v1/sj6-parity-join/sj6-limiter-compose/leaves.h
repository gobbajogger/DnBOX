static int16_t old_sj6_room_pcm(float x)
{
    if(x>1.f)x=1.f;
    if(x< -1.f)x=-1.f;
    return (int16_t)(32767.f*x);
}
static void old_sj6_room_tick(sj6_room_t *s)
{
    /* Two compact room responses: paired asymmetric combs and a four-line
     * diffusing network. Neither stores or invokes the app's original IR. */
    static const uint8_t len[2][4]={{47,59,73,89},{SJ6_ROOM_L0,SJ6_ROOM_L1,SJ6_ROOM_L2,SJ6_ROOM_L3}};
    static const uint16_t base[4]={0,SJ6_ROOM_L0,SJ6_ROOM_L0+SJ6_ROOM_L1,
        SJ6_ROOM_L0+SJ6_ROOM_L1+SJ6_ROOM_L2};
    const uint8_t *length=len[s->type];
    float x[4],in[2],feedback[4];
    for(unsigned c=0;c<2;++c){
        float y=s->acc[c]*(1.f/SJ6_ROOM_STRIDE);
        s->acc[c]=0.f;
        s->lo[c]+=s->hp_coeff*(y-s->lo[c]);
        in[c]=y-s->lo[c];
        s->last[c]=s->wet[c];
    }
    for(unsigned i=0;i<4;++i)x[i]=(1.f/32767.f)*s->ring[base[i]+s->pos[i]];
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
        s->ring[base[i]+s->pos[i]]=old_sj6_room_pcm(.42f*in[i&1u]+s->fb*feedback[i]);
        if(++s->pos[i]>=length[i])s->pos[i]=0;
    }
}
/* Build9 Fatten stereo topology. The output of the delayed left leg and
 * the dry right leg are both discarded by the hard panners. Only right
 * history is needed. The compact 15 kHz feedback LP is Butterworth;
 * AudioUnit private internals are not assumed sample-identical. */
static float old_sj6_fatten_tick(sj6_room_t *s,float right)
{
    const unsigned at=s->wide_pos;
    const float delayed=(4.f/32767.f)*(float)s->wide[at];
    const float x=right+.5f*delayed;
    const float y=0.481161993f*x+s->fat_lp[0];
    s->fat_lp[0]=0.962323986f*x-0.672069140f*y+s->fat_lp[1];
    s->fat_lp[1]=0.481161993f*x-0.252578833f*y;
    s->wide[at]=old_sj6_room_pcm(.25f*y);
    s->wide_pos=at+1u==SJ6_ROOM_WIDE?0u:(uint16_t)(at+1u);
    return delayed;
}
static void old_room(sj6_room_t *s,float *left,float *right,unsigned frames)
{
    if(!s->armed||(!s->enable&&s->armed!=2&&!s->widen&&s->depth==0.f))return;
    for(unsigned i=0;i<frames;++i){
        float l=left[i],r=right[i];
        if(s->enable||s->armed==2){
            if(s->enable){s->acc[0]+=l;s->acc[1]+=r;}
            if(++s->tick==SJ6_ROOM_STRIDE){s->tick=0;old_sj6_room_tick(s);}
            if(s->mix>0.f){
                float t=(float)s->tick*(1.f/SJ6_ROOM_STRIDE);
                /* Type II is a parallel return: retain dry even before the
                 * first reflection or while the inlet is closed. This
                 * diagnostic leaves the wet scale and kernel unchanged. */
                const float wl=s->last[0]+t*(s->wet[0]-s->last[0]);
                const float wr=s->last[1]+t*(s->wet[1]-s->last[1]);
                l+=(s->type?wl:wl-l)*s->mix;
                r+=(s->type?wr:wr-r)*s->mix;
            }
        }
        if(s->widen)r=old_sj6_fatten_tick(s,r);
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
static void old_limit(sj6_limiter_t *s,float *l,float *r,unsigned frames)
{
    if(!s->armed)return;
    float gain=s->gain;
    for(unsigned i=0;i<frames;++i){
        const float peak=fm_maxf(fm_fabsf(l[i]),fm_fabsf(r[i]));
        float target=1.f;
        if(peak>s->threshold){
            const float over=peak-s->threshold;
            const float soft=s->threshold+over/(1.f+s->inverse_knee*over);
            target=soft/peak;
        }
        /* Stereo-linked gain changes, true attack, fixed compact recovery.
         * Original Bonus Release label invokes ATTACK, not this recovery. */
        gain+=(target-gain)*(target<gain?s->attack:.0004534119f);
        const float applied=s->output_gain*gain;
        l[i]*=applied;r[i]*=applied;
    }
    s->gain=gain;
}
static inline float sj6_limiter_sample(sj6_limiter_t *s,float *l,float *r,float gain)
{
        const float peak=fm_maxf(fm_fabsf(*l),fm_fabsf(*r));
        float target=1.f;
        if(peak>s->threshold){
            const float over=peak-s->threshold;
            const float soft=s->threshold+over/(1.f+s->inverse_knee*over);
            target=soft/peak;
        }
        /* Stereo-linked gain changes, true attack, fixed compact recovery.
         * Original Bonus Release label invokes ATTACK, not this recovery. */
        gain+=(target-gain)*(target<gain?s->attack:.0004534119f);
        const float applied=s->output_gain*gain;
        *l*=applied;*r*=applied;
        return gain;
}
static void sj6_limiter_process(sj6_limiter_t *s,float *l,float *r,unsigned frames)
{
    if(!s->armed)return;
    float gain=s->gain;
    for(unsigned i=0;i<frames;++i)gain=sj6_limiter_sample(s,l+i,r+i,gain);
    s->gain=gain;
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
    static const uint8_t len[2][4]={{47,59,73,89},{SJ6_ROOM_L0,SJ6_ROOM_L1,SJ6_ROOM_L2,SJ6_ROOM_L3}};
    static const uint16_t base[4]={0,SJ6_ROOM_L0,SJ6_ROOM_L0+SJ6_ROOM_L1,
        SJ6_ROOM_L0+SJ6_ROOM_L1+SJ6_ROOM_L2};
    const uint8_t *length=len[s->type];
    float x[4],in[2],feedback[4];
    for(unsigned c=0;c<2;++c){
        float y=s->acc[c]*(1.f/SJ6_ROOM_STRIDE);
        s->acc[c]=0.f;
        s->lo[c]+=s->hp_coeff*(y-s->lo[c]);
        in[c]=y-s->lo[c];
        s->last[c]=s->wet[c];
    }
    for(unsigned i=0;i<4;++i)x[i]=(1.f/32767.f)*s->ring[base[i]+s->pos[i]];
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
        s->ring[base[i]+s->pos[i]]=sj6_room_pcm(.42f*in[i&1u]+s->fb*feedback[i]);
        if(++s->pos[i]>=length[i])s->pos[i]=0;
    }
}
/* Build9 Fatten stereo topology. The output of the delayed left leg and
 * the dry right leg are both discarded by the hard panners. Only right
 * history is needed. The compact 15 kHz feedback LP is Butterworth;
 * AudioUnit private internals are not assumed sample-identical. */
static float sj6_fatten_tick(sj6_room_t *s,float right)
{
    const unsigned at=s->wide_pos;
    const float delayed=(4.f/32767.f)*(float)s->wide[at];
    const float x=right+.5f*delayed;
    const float y=0.481161993f*x+s->fat_lp[0];
    s->fat_lp[0]=0.962323986f*x-0.672069140f*y+s->fat_lp[1];
    s->fat_lp[1]=0.481161993f*x-0.252578833f*y;
    s->wide[at]=sj6_room_pcm(.25f*y);
    s->wide_pos=at+1u==SJ6_ROOM_WIDE?0u:(uint16_t)(at+1u);
    return delayed;
}
static void sj6_room_process(sj6_room_t *s,float *left,float *right,unsigned frames
#if X0X_SJ6_NATIVE_LIMITER
                             ,sj6_limiter_t *dry_limit
#endif
                             )
{
    if(!s->armed||(!s->enable&&s->armed!=2&&!s->widen&&s->depth==0.f))return;
    for(unsigned i=0;i<frames;++i){
        float l=left[i],r=right[i];
        if(s->enable||s->armed==2){
            if(s->enable){s->acc[0]+=l;s->acc[1]+=r;}
#if X0X_SJ6_NATIVE_LIMITER
            /* The room feed is from post-delay before the dry compressor.
             * This opt-in is admitted only for the existing additive return. */
            if(dry_limit)dry_limit->gain=sj6_limiter_sample(dry_limit,&l,&r,dry_limit->gain);
#endif
            if(++s->tick==SJ6_ROOM_STRIDE){s->tick=0;sj6_room_tick(s);}
            if(s->mix>0.f){
                float t=(float)s->tick*(1.f/SJ6_ROOM_STRIDE);
                /* Type II is a parallel return: retain dry even before the
                 * first reflection or while the inlet is closed. This
                 * diagnostic leaves the wet scale and kernel unchanged. */
                const float wl=s->last[0]+t*(s->wet[0]-s->last[0]);
                const float wr=s->last[1]+t*(s->wet[1]-s->last[1]);
                l+=(s->type?wl:wl-l)*s->mix;
                r+=(s->type?wr:wr-r)*s->mix;
            }
        }
        if(s->widen)r=sj6_fatten_tick(s,r);
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
