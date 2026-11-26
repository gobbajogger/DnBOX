/* A common bit-identical L/R/voice-send gain recurrence. No history alias:
 * only equal live lane states are co-rendered and all original states commit. */
static inline float fm1_909_mix_common(float *restrict left,float *restrict right,
    float *restrict rev,float *restrict dly,unsigned n,float *restrict current,float target,
    float level,float pk)
{
    float c=*current;
    const int moving=c!=target;
    for(unsigned i=0;i<n;i++){
        const float l=left[i],r=right[i],m=(l+r)*.5f;
        if(moving)c+=(target-c)*GLIDE_A;
        left[i]=l*c;right[i]=r*c;
        rev[i]*=c;dly[i]*=c;
        float a=fm_fabsf(m)*level;
        if(a>pk)pk=a;
    }
    if(moving && fm_fabsf(target-c)<1e-5f)c=target;
    *current=c;
    return pk;
}
static inline unsigned fm1_mix_bits(float f){union{float f;unsigned u;}s={.f=f};return s.u;}
