// Generated morph-only direct-read derivative. See DERIVATION.json.
#include "morph_compute.h"
#include "float_math.h"
#include <string.h>
int s1_runtime_morph_compute(sp_data *sp, sp_oscmorph2d *osc, SPFLOAT *in, SPFLOAT *out, const s1_morph_binding *binding)
{
    if(!sp||!osc||!out||!s1_morph_binding_valid(binding)||!osc->tbl||!osc->tbl[0]||!osc->fbl||osc->nft!=4||osc->nbl!=13||!s1_finitef(osc->freq)||osc->freq<0.f||!s1_finitef(osc->amp)||osc->lphs<0||(uint32_t)osc->lphs>SP_FT_PHMASK) return SP_NOT_OK;
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
            bandlimitIndex = s1_ifloor(bandlimitIndexOverride);
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
    
    osc->inc = (int32_t)s1_lrintf(cps * sicvt);  

    fract = ((phs) & ftp1->lomask) * ftp1->lodiv;

    pos = phs >> lobits;

    uint32_t w11,w21,w12,w22;
    SPFLOAT f11,f21,f12,f22;
    if(!s1_morph_read_word(binding,(unsigned)row1,(unsigned)pos,&w11)||
       !s1_morph_read_word(binding,(unsigned)row2,(unsigned)pos,&w21)||
       !s1_morph_read_word(binding,(unsigned)row1,(unsigned)((pos+1)%ftp1->size),&w12)||
       !s1_morph_read_word(binding,(unsigned)row2,(unsigned)((pos+1)%ftp1->size),&w22)) return SP_NOT_OK;
    memcpy(&f11,&w11,4);memcpy(&f21,&w21,4);memcpy(&f12,&w12,4);memcpy(&f22,&w22,4);

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
