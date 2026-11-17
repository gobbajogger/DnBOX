#if defined(S1_HARMONIC_ONLY)
// Generated morph-only direct-read derivative. See DERIVATION.json.
#include "compact_morph.h"
#include "float_math.h"
#include <string.h>
#include "harmonic.h"
int s1c_runtime_morph_compute(sp_data *sp, sp_oscmorph2d *osc, SPFLOAT *in, SPFLOAT *out, const s1c_binding *binding)
{
    if(!sp||!osc||!out||(!binding||binding->bytes||binding->byte_count||binding->format_tag!=0x48334d31u)||!osc->tbl||!osc->tbl[0]||!osc->fbl||osc->nft!=4||osc->nbl!=13||!s1_finitef(osc->freq)||osc->freq<0.f||!s1_finitef(osc->amp)||osc->lphs<0||(uint32_t)osc->lphs>SP_FT_PHMASK) return SP_NOT_OK;
    sp_ftbl *ftp1;
    SPFLOAT amp, cps, fract, v1, v2;
    int row1, row2;
    int32_t phs, lobits, pos;
    SPFLOAT sicvt = osc->tbl[0]->sicvt; /* sicvt: this stands for Sampling Increment ConVert */
    const int enableBandlimit = osc->enableBandlimit;
    int bandlimitIndexOverride = osc->bandlimitIndexOverride;

    int32_t bandlimitIndex = 0;
    if (enableBandlimit > 0) {
        if (bandlimitIndexOverride < 0) {
            /* do not use override */
            for(int i = 1; i < osc->nbl; i++) {
                if(osc->freq <= osc->fbl[i]) {
                    bandlimitIndex = i;
                    break;
                }
            }
        } else {
            /* the override is the index */
            bandlimitIndex = bandlimitIndexOverride;
            if (bandlimitIndex < 0)
                bandlimitIndex = 0;
        }
    } else {
        /* hard-code the "non-bandlimited" row of wavetables */
        bandlimitIndex = 0;
    }

    if(bandlimitIndex<0||bandlimitIndex>=13||!s1_finitef(osc->wtpos)||osc->wtpos<0.f||osc->wtpos>=2147483648.f) return SP_NOT_OK;
    /* Use only the fractional part of the position or 1 */
    if (osc->wtpos > 1.f) {
        osc->wtpos -= (int)osc->wtpos;
    }
    
    const SPFLOAT findex = (bandlimitIndex * osc->nft) + osc->wtpos * (osc->nft - 1);
    const int index = s1_ifloor(findex);
    const SPFLOAT wtfrac = findex - index;
    if(index<0||index>=52||!osc->tbl[index]||osc->tbl[index]->size!=4096||osc->tbl[index]->lobits>=31) return SP_NOT_OK;

    lobits = osc->tbl[index]->lobits;
    amp = osc->amp;
    cps = osc->freq;
    phs = osc->lphs;
    ftp1 = osc->tbl[index];
    row1 = index;

    if(index >= (bandlimitIndex * osc->nft) + (osc->nft - 1)) {
        row2 = row1;
    } else {
        row2 = index + 1;
    }
    
    osc->inc = s1_lrintf(cps * sicvt);  

    fract = ((phs) & ftp1->lomask) * ftp1->lodiv;

    pos = phs >> lobits;

    /* Preserve explicit source band selection/override and original interpolation.
     * Bandlimited rows are approximated by a transition width at their source
     * band ceiling. This is deliberately measured, not assumed equivalent. */
    static const unsigned counts[13]={0,2696,1348,674,337,168,84,42,21,10,5,2,1};
    unsigned harmonics=counts[bandlimitIndex];
    const unsigned next=(unsigned)((pos+1)%ftp1->size);
    SPFLOAT f11=reuse_wave((unsigned)row1%4,(unsigned)pos,harmonics);
    SPFLOAT f21=reuse_wave((unsigned)row2%4,(unsigned)pos,harmonics);
    SPFLOAT f12=reuse_wave((unsigned)row1%4,next,harmonics);
    SPFLOAT f22=reuse_wave((unsigned)row2%4,next,harmonics);

    v1 = (1 - wtfrac) * 
        f11 + 
        wtfrac * 
        f21;
    v2 = (1 - wtfrac) * 
        f12+ 
        wtfrac * 
        f22;

    *out = (v1 + (v2 - v1) * fract) * amp;

    phs += osc->inc;
    phs &= SP_FT_PHMASK;

    osc->lphs = phs;
    return SP_OK;
}

#else
// Generated morph-only direct-read derivative. See DERIVATION.json.
#include "compact_morph.h"
#include "float_math.h"
#include <string.h>
int s1c_runtime_morph_compute(sp_data *sp, sp_oscmorph2d *osc, SPFLOAT *in, SPFLOAT *out, const s1c_binding *binding)
{
    if(!sp||!osc||!out||!s1c_binding_valid(binding)||!osc->tbl||!osc->tbl[0]||!osc->fbl||osc->nft!=4||osc->nbl!=13||!s1_finitef(osc->freq)||osc->freq<0.f||!s1_finitef(osc->amp)||osc->lphs<0||(uint32_t)osc->lphs>SP_FT_PHMASK) return SP_NOT_OK;
    sp_ftbl *ftp1;
    SPFLOAT amp, cps, fract, v1, v2;
    int row1, row2;
    int32_t phs, lobits, pos;
    SPFLOAT sicvt = osc->tbl[0]->sicvt; /* sicvt: this stands for Sampling Increment ConVert */
    const int enableBandlimit = osc->enableBandlimit;
    int bandlimitIndexOverride = osc->bandlimitIndexOverride;

    int32_t bandlimitIndex = 0;
    if (enableBandlimit > 0) {
        if (bandlimitIndexOverride < 0) {
            /* do not use override */
            for(int i = 1; i < osc->nbl; i++) {
                if(osc->freq <= osc->fbl[i]) {
                    bandlimitIndex = i;
                    break;
                }
            }
        } else {
            /* the override is the index */
            bandlimitIndex = bandlimitIndexOverride;
            if (bandlimitIndex < 0)
                bandlimitIndex = 0;
        }
    } else {
        /* hard-code the "non-bandlimited" row of wavetables */
        bandlimitIndex = 0;
    }

    if(bandlimitIndex<0||bandlimitIndex>=13||!s1_finitef(osc->wtpos)||osc->wtpos<0.f||osc->wtpos>=2147483648.f) return SP_NOT_OK;
    /* Use only the fractional part of the position or 1 */
    if (osc->wtpos > 1.f) {
        osc->wtpos -= (int)osc->wtpos;
    }
    
    const SPFLOAT findex = (bandlimitIndex * osc->nft) + osc->wtpos * (osc->nft - 1);
    const int index = s1_ifloor(findex);
    const SPFLOAT wtfrac = findex - index;
    if(index<0||index>=52||!osc->tbl[index]||osc->tbl[index]->size!=4096||osc->tbl[index]->lobits>=31) return SP_NOT_OK;

    lobits = osc->tbl[index]->lobits;
    amp = osc->amp;
    cps = osc->freq;
    phs = osc->lphs;
    ftp1 = osc->tbl[index];
    row1 = index;

    if(index >= (bandlimitIndex * osc->nft) + (osc->nft - 1)) {
        row2 = row1;
    } else {
        row2 = index + 1;
    }
    
    osc->inc = s1_lrintf(cps * sicvt);  

    fract = ((phs) & ftp1->lomask) * ftp1->lodiv;

    pos = phs >> lobits;

    uint32_t words[4];
    SPFLOAT f11,f21,f12,f22;
    if(!s1c_read_quad(binding,(unsigned)row1,(unsigned)row2,(unsigned)pos,
                     (unsigned)((pos+1)%ftp1->size),words)) return SP_NOT_OK;
    __builtin_memcpy(&f11,words,4);__builtin_memcpy(&f21,words+1,4);__builtin_memcpy(&f12,words+2,4);__builtin_memcpy(&f22,words+3,4);

    v1 = (1 - wtfrac) * 
        f11 + 
        wtfrac * 
        f21;
    v2 = (1 - wtfrac) * 
        f12+ 
        wtfrac * 
        f22;

    *out = (v1 + (v2 - v1) * fract) * amp;

    phs += osc->inc;
    phs &= SP_FT_PHMASK;

    osc->lphs = phs;
    return SP_OK;
}

#endif
