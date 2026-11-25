/* SPDX-License-Identifier: GPL-3.0-only */
/* Compact stereo SuperJ6 Delay 24–27 + Sync 118. The caller loans an
 * engine-exclusive, owner-local ring. At long taps the history is sampled
 * more slowly; neither the time law nor its UI/persisted detents change. */
#if X0X_SJ6_NATIVE_DELAY
enum { SJ6_NATIVE_DELAY_MAX_STRIDE = 8192 };

static float native_delay_frames(const juno6_t *s,unsigned raw,float bpm,unsigned sync)
{
    static const uint16_t grid128[18]={
        0,3,4,6,8,12,16,24,32,48,64,96,128,256,384,512,768,1024
    }; /* fractions of a whole note /128, same original 18 detents */
    if (raw>127u || sync>1u || !(bpm>0.f) || bpm>10000.f) return 0.f;
    unsigned i=sync?(raw*17u+63u)/127u:0u;
    float sec=i ? (float)grid128[i]*(1.875f/bpm) : (1.5f/127.f)*(float)raw;
    float frames=sec*s->sample_rate;
    if (!(frames<=(SJ6_NATIVE_DELAY_CAP-2u)*SJ6_NATIVE_DELAY_MAX_STRIDE)) return 0.f;
    return frames<1.f ? 1.f : frames;
}

void juno6_delay_native_arm(juno6_t *s,int16_t *history)
{
    if (s->typed && !s->delay24.tap) s->delay24.tap=history;
}

int juno6_delay25_can(const juno6_t *s,unsigned raw,float bpm,unsigned sync)
{
    return s->delay24.tap && native_delay_frames(s,raw,bpm,sync)>0.f;
}

/* Match the existing original control ramp cadence: 10,000 frames,
 * update every eight rendered frames. No per-sample division. */
static void native_delay_ramp(float current,float next,float *target,float *step,uint16_t *ramp)
{
    if (*target==next) return;
    *target=next;
    *step=(next-current)*(1.f/1250.f);
    *ramp=1250u;
}
static void native_delay_advance(float *current,float target,float step,uint16_t *ramp)
{
    if (*ramp) *current=--*ramp ? *current+step : target;
}

#if X0X_SJ6_AUX_LFO_NATIVE
/* When any of the original LFO selectors targets Delay Time, provision the
 * whole reachable compact span once. A control tick then selects a tap
 * without clearing or changing sample rate mid-tail. */
static float native_delay_route_window(const juno6_t *s,float bpm)
{
    int owned=0;
    for(unsigned k=0;k<SJ6_AUX_FX_ROUTES;k++)
        if(sj6_aux_destination_id(juno6_aux_fx_route(s,k))==25){owned=1;break;}
    if(!owned)return 0.f;
    float max=native_delay_frames(s,127u,bpm,s->delay24.sync);
    if(max>0.f)return max;
    unsigned lo=0,hi=127u;
    while(hi-lo>1u){
        const unsigned mid=(lo+hi)>>1;
        if(native_delay_frames(s,mid,bpm,s->delay24.sync)>0.f)lo=mid;else hi=mid;
    }
    return native_delay_frames(s,lo,bpm,s->delay24.sync);
}

/* In-place coarsening aligned on the newest complete old cell. Write
 * indices trail the already-read source indices, including across wrap. */
static void native_delay_route_coarsen(juno6_t *s,unsigned wanted)
{
    const unsigned old=s->delay24.stride;
    const unsigned ratio=wanted/old;
    const unsigned filled=s->delay24.filled/ratio;
    const unsigned pos=s->delay24.pos;
    enum { MASK=SJ6_NATIVE_DELAY_CAP-1u };
    for(unsigned i=0;i<filled;i++){
        int32_t l=0,r=0;
        for(unsigned j=0;j<ratio;j++){
            unsigned from=((pos-1u-i*ratio-j)&MASK)*2u;
            l+=s->delay24.tap[from];r+=s->delay24.tap[from+1u];
        }
        const unsigned to=((pos-1u-i)&MASK)*2u;
        s->delay24.tap[to]=(int16_t)(l/(int32_t)ratio);
        s->delay24.tap[to+1u]=(int16_t)(r/(int32_t)ratio);
    }
    s->delay24.filled=(uint16_t)filled;
    s->delay24.stride=(uint16_t)wanted;
    s->delay24.inv_stride=1.f/(float)wanted;
    const float convert=(float)old*s->delay24.inv_stride;
    s->delay24.current*=convert;
    s->delay24.target*=convert;
    s->delay24.step[0]*=convert;
    /* The existing partial bin stays pending; phase < old <= wanted,
       and both channels already hold the exact uncommitted time sum. */
}
void juno6_delay_native_route_refresh(juno6_t *s,float bpm)
{
    if(!s->delay24.tap || !s->delay24.stride || !(bpm>0.f))return;
    const float window=native_delay_route_window(s,bpm);
    unsigned stride=s->delay24.stride;
    while(window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride))stride<<=1;
    if(stride!=s->delay24.stride)native_delay_route_coarsen(s,stride);
}
#endif

void juno6_delay_native_config(juno6_t *s,const uint8_t b[4],float bpm)
{
    if (!s->delay24.tap) return;
    float frames=native_delay_frames(s,b[1],bpm,!!(b[0]&2u));
    s->delay24.clock_refused=!(frames>0.f);
    s->delay24.sync=(uint8_t)!!(b[0]&2u);
    s->delay24.enabled=b[0]&1u;
    if (!(frames>0.f)) return;
    /* Power of two bins permit stable repeats, and hysteresis minimizes the
     * only non-transplantable edit: switching the history sample rate. */
    unsigned stride=s->delay24.stride;
    float window=frames;
#if X0X_SJ6_AUX_LFO_NATIVE
    const float route=native_delay_route_window(s,bpm);
    if(route>window)window=route;
    /* Never shrink/reset a warm, routed history on unrelated amount edits. */
    if (route>0.f && stride && window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride)){
        while(window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride))stride<<=1;
        native_delay_route_coarsen(s,stride);
    }
#else
    const float route=0.f;
#endif
    if (!stride || window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride) ||
        (route==0.f && stride>1u && frames<(float)((SJ6_NATIVE_DELAY_CAP/4u)*stride))) {
        stride=1u;
        while (window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride)) stride<<=1;
        if (stride!=s->delay24.stride) {
            s->delay24.stride=(uint16_t)stride;
            s->delay24.filled=s->delay24.phase=s->delay24.pos=0;
            s->delay24.sum[0]=s->delay24.sum[1]=0.f;
            s->delay24.inv_stride=1.f/(float)stride;
            s->delay24.current=s->delay24.target=frames*s->delay24.inv_stride;
            s->delay24.ramp[0]=0;
        }
    }
    native_delay_ramp(s->delay24.current,frames*s->delay24.inv_stride,&s->delay24.target,
                      &s->delay24.step[0],&s->delay24.ramp[0]);
    native_delay_ramp(s->delay24.fb,(.99f/127.f)*b[2],&s->delay24.fb_target,
                      &s->delay24.step[1],&s->delay24.ramp[1]);
    unsigned mix=(b[0]&1u)?b[3]:0u;
    if (mix!=s->delay24.mix) {
        s->delay24.mix=(uint8_t)mix;
        native_delay_ramp(s->delay24.wet,mix ? .9f*fm_powf((float)mix/127.f,1.1f) : 0.f,
                          &s->delay24.wet_target,&s->delay24.step[2],&s->delay24.ramp[2]);
    }
}

float juno6_aux_delay_frames(const juno6_t *s,unsigned raw)
{
    float v=native_delay_frames(s,raw,s->lfo_tempo,s->delay24.sync);
    if(v>0.f && s->delay24.stride && v<=(SJ6_NATIVE_DELAY_CAP-2u)*s->delay24.stride)
        return v*s->delay24.inv_stride;
    return 0.f; /* No wrap into stale/unaligned history at this stride. */
}
static int16_t native_delay_encode(float x)
{
    return x>=1.f?32767:x<=-1.f?-32767:(int16_t)(x*32767.f);
}

void juno6_delay24_process(juno6_t *s,float *left,float *right,unsigned n)
{
    if (!s->delay24.stride || !s->delay24.tap) return;
    enum { MASK=SJ6_NATIVE_DELAY_CAP-1u };
    if (n && s->delay24.stride==1u && s->delay24.current==1.f && s->delay24.target==1.f &&
        s->delay24.fb==0.f && s->delay24.fb_target==0.f && s->delay24.wet==0.f && s->delay24.wet_target==0.f) {
        /* Default, dry 1-sample path: only the previous input can be used. */
        unsigned pos=s->delay24.pos;
        s->delay24.tap[2u*pos]=native_delay_encode(left[n-1u]);
        s->delay24.tap[2u*pos+1u]=native_delay_encode(right[n-1u]);
        s->delay24.pos=(uint16_t)((pos+1u)&MASK);
        s->delay24.filled=1;
        return;
    }
    for (unsigned i=0;i<n;++i) {
        if (!(s->delay24.tick++ & 7u)) {
            native_delay_advance(&s->delay24.current,s->delay24.target,s->delay24.step[0],&s->delay24.ramp[0]);
            native_delay_advance(&s->delay24.fb,s->delay24.fb_target,s->delay24.step[1],&s->delay24.ramp[1]);
            native_delay_advance(&s->delay24.wet,s->delay24.wet_target,s->delay24.step[2],&s->delay24.ramp[2]);
        }
        float age=s->delay24.current+1.f-((float)s->delay24.phase+1.f)*s->delay24.inv_stride;
        if (age<1.f) age=1.f;
        unsigned whole=(unsigned)age, pos=s->delay24.pos;
        unsigned a=((pos-whole)&MASK)*2u, b=((pos-whole-1u)&MASK)*2u;
        float delay[2]={0.f,0.f};
        if (whole<=s->delay24.filled) {
            float frac=age-(float)whole;
            const int16_t *history=s->delay24.tap;
            for (unsigned c=0;c<2;++c) {
                float x=history[a+c], y=whole<s->delay24.filled ? history[b+c] : 0.f;
                delay[c]=(x+(y-x)*frac)*(1.f/32767.f);
            }
        }
        s->delay24.sum[0]+=left[i]+s->delay24.fb*delay[0];
        s->delay24.sum[1]+=right[i]+s->delay24.fb*delay[1];
        if (++s->delay24.phase==s->delay24.stride) {
            for (unsigned c=0;c<2;++c) {
                s->delay24.tap[2u*pos+c]=native_delay_encode(s->delay24.sum[c]*s->delay24.inv_stride);
                s->delay24.sum[c]=0.f;
            }
            s->delay24.phase=0;
            s->delay24.pos=(uint16_t)((pos+1u)&MASK);
            if (s->delay24.filled<SJ6_NATIVE_DELAY_CAP) ++s->delay24.filled;
        }
        if (!s->delay24.clock_refused && s->delay24.wet!=0.f) {
            float dry=1.f-s->delay24.wet;
            left[i]=dry*left[i]+s->delay24.wet*delay[0];
            right[i]=dry*right[i]+s->delay24.wet*delay[1];
        }
    }
}
#endif
