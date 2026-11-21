/* SPDX-License-Identifier: GPL-3.0-only */
/* The standalone 26,112-cell private delay from the e3ffb3d8 experiment.
 * Build in the fxbus.c translation unit, using its existing 8W8 constants,
 * biquad, denormal handling and saturating cell writer. */
void fxbus_808_delay(fxbus_t *f,const float *send,float *left,float *right,int frames,float gain)
{
    const float target=f->dl_time_ms*.001f*D9_SR,level=f->dl_level*gain;
    unsigned w=(unsigned)f->w;
    if(f->dl_quiet>=D8_QUIET){
        int i=0;while(send && i<frames && send[i]==0)++i;
        if(!send || i==frames){
            for(i=0;i<frames;i++){
                float next=f->dcur+(target-f->dcur)*.0008f;if(next==f->dcur)break;f->dcur=next;
            }
            f->w=(w+(unsigned)frames)%FX_808_DELAY_FRAMES;
            return;
        }
    }
    d9_biquad_t hp=f->dl_hp;
    float lp=f->lp, dcur=f->dcur;
    const float feedback=f->dl_fdbk,tone=f->dl_tc;
    int quiet=f->dl_quiet;
    /* At this magnitude the delay is a multiple of 1/512 sample. Subtracting
     * any write index and wrapping within this ring are then exact in binary32.
     * A settled tap advances by exactly one cell on every sample, retaining
     * the same interpolation fraction until the pot or tempo changes. */
    if (dcur >= 16384.f && dcur + (target-dcur)*.0008f == dcur) {
        float pos = (float)w-dcur;
        if (pos < 0) pos += FX_808_DELAY_FRAMES;
        unsigned a = (unsigned)pos;
        float frac = pos-(float)a;
        if (a == FX_808_DELAY_FRAMES) a = 0;
        for (int offset = 0; offset < frames;) {
            unsigned b = a + 1 == FX_808_DELAY_FRAMES ? 0 : a + 1;
            unsigned span = (unsigned)(frames-offset);
            if (span > FX_808_DELAY_FRAMES-b) span = FX_808_DELAY_FRAMES-b;
            if (span > FX_808_DELAY_FRAMES-w) span = FX_808_DELAY_FRAMES-w;
            int prior = f->dbuf[a];
            for (unsigned j=0; j<span; ++j) {
                unsigned i=(unsigned)offset+j;
                int next=f->dbuf[b+j];
                float y=((float)prior+(float)(next-prior)*frac)*(1.f/2048);
                float input=send?send[i]:0;
                float x=d9_biquad_tick(&hp,input);
                lp+=(y*feedback-lp)*tone;
                if(fm_fabsf(lp)<1e-20f)lp=0;
                int16_t cell;
                sat12_store(&cell,x+lp);
                f->dbuf[w+j]=cell;
                left[i]+=y*level;
                if(right)right[i]+=y*level;
                quiet=!cell && !input?quiet+1:0;
                prior=next;
            }
            a=b+span-1;
            w+=span;
            if(w==FX_808_DELAY_FRAMES)w=0;
            offset+=(int)span;
        }
    } else for(int i=0;i<frames;i++){
        float input=send?send[i]:0,x=d9_biquad_tick(&hp,input);
        dcur+=(target-dcur)*.0008f;
        float pos=(float)w-dcur;if(pos<0)pos+=FX_808_DELAY_FRAMES;
        unsigned a=(unsigned)pos;float frac=pos-(float)a;if(a>=FX_808_DELAY_FRAMES)a-=FX_808_DELAY_FRAMES;
        unsigned b=a+1==FX_808_DELAY_FRAMES?0:a+1;
        float u=(float)f->dbuf[a]*(1.f/2048),v=(float)f->dbuf[b]*(1.f/2048);
        float y=u+(v-u)*frac;
        lp+=(y*feedback-lp)*tone;
        if(fm_fabsf(lp)<1e-20f)lp=0;
        int16_t cell;sat12_store(&cell,x+lp);
        f->dbuf[w]=cell;
        if(++w==FX_808_DELAY_FRAMES)w=0;
        left[i]+=y*level;if(right)right[i]+=y*level;quiet=!cell && !input?quiet+1:0;
    }
    f->dl_hp=hp;f->lp=lp;f->dcur=dcur;f->dl_quiet=quiet;
    f->w=(int32_t)w;
}
