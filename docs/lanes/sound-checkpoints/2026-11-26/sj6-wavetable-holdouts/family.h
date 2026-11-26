#include "selector.h"
#include "family-data.h"
/* Low-frequency group delay of the TWO verified F32 allpass decimation
 * stages from independent current source. D is derived analytically at DC
 * from coefficients (not fit to any note), so this still will not exactly
 * match nonlinear high-frequency phase or initial zero-state transient. */
#define SOURCE_GROUP_DELAY .97079264196f
#ifdef TBL_DEDUP
#define TABLE_AT(s,b) TBL[TBL_IDX[(s)][(b)]]
#else
#define TABLE_AT(s,b) TBL[(s)][(b)]
#endif
static inline float sj6_family_interpolate(const void *bank,unsigned phase,unsigned next,float frac)
{
#if FAMILY_WORD == 8
    const int8_t *w=bank;
#else
    const int16_t *w=bank;
#endif
    float a=(float)w[phase],b=(float)w[next];
    return a+frac*(b-a);
}
static inline float sj6_family(unsigned shape,float phase,float step)
{
    const sj6_band band=sj6_wavetable_band(step);
    phase-=SOURCE_GROUP_DELAY*step;
    if(phase<0.f)phase+=1.f;
    float idx=phase*TBL_N;
    unsigned head=(unsigned)idx,next=(head+1u)&(TBL_N-1u);
    float frac=idx-(float)head;
    head&=TBL_N-1u;
    float a=sj6_family_interpolate(TABLE_AT(shape,band.row),head,next,frac);
    if(band.blend>0.f){
        float b=sj6_family_interpolate(TABLE_AT(shape,band.row+1u),head,next,frac);
        a+=band.blend*(b-a);
    }
    /* Existing independent raw constructor profiles and mathematical H1
     * shape amplitudes. No end-to-end gain correction. */
    const float amp=shape?.495f*1.17878819f*(8.f/(3.14159265358979323846f*3.14159265358979323846f)):.5f*1.2377909421920776f;
    return a*(amp*TBL_GAIN[shape]);
}
