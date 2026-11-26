/* Settled glides only. A separate stack-independent bitwise host guard uses
 * precisely this loop. Original fallback remains for every moving glide. */
static inline float fm1_909_mix_settled(float *restrict left,float *restrict right,
    float *restrict rev,float *restrict dly,unsigned n,const float g[6],float level,float pk)
{
    for(unsigned i=0;i<n;i++){
        const float l=left[i],r=right[i],m=(l+r)*.5f;
        left[i]=l*g[0];
        right[i]=r*g[1];
        float rv=rev[i],dl=dly[i];
        rv*=g[4];
        dl*=g[5];
        if(g[2]!=0.f)rv+=m*g[2];
        if(g[3]!=0.f)dl+=m*g[3];
        rev[i]=rv;dly[i]=dl;
        float a=fm_fabsf(m)*level;
        if(a>pk)pk=a;
    }
    return pk;
}
