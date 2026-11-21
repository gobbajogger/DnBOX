/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "soundpipe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* original-stage.inc contains the unmodified LFO, bitcrush and tremolo
 * expressions from S1DSPKernel+process.mm. */
enum {
    lfo1Index, lfo1Amplitude, lfo1Rate, lfo2Index, lfo2Amplitude, lfo2Rate,
    bitCrushSampleRate, bitcrushLFO, tremoloLFO
};
static float parameters[9];
static sp_data original_sp = {.sr = 44100, .nchan = 2};
static sp_data *sp = &original_sp;
static sp_phasor phasors[2], *lfo1Phasor = phasors, *lfo2Phasor = phasors+1;
static sp_port ports[2], *lfo1Port = ports, *lfo2Port = ports+1;
static float bitcrushIncr = 1.f, bitcrushIndex, bitcrushSampleIndex, bitcrushValue;
static float lfo1Smooth, lfo2Smooth, lfo1_0_1, lfo1_1_0, lfo2_0_1, lfo2_1_0, lfo3_0_1, lfo3_1_0;
#define nil NULL
static int sampleRate(void) { return 44100; }
static float clampedValue(unsigned id, float value)
{ assert(id == bitCrushSampleRate); return fminf(48000.f, fmaxf(2048.f,value)); }
static float original_stage(float synthOut)
{
#include "original-stage.inc"
    return bitCrushOut;
}

static original_runtime_part dry, wet;
static original_runtime_workspace scratch;

static void note(unsigned key, unsigned velocity)
{
    assert(original_runtime_key(&dry, ORIGINAL_INPUT_KEYS,key,velocity) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&wet, ORIGINAL_INPUT_KEYS,key,velocity) == ORIGINAL_RUNTIME_OK);
}

int main(int argc, char **argv)
{
    assert(argc==4 || argc==5);
    unsigned index=(unsigned)strtoul(argv[2],NULL,10);
    FILE *input=fopen(argv[1],"rb");
    uint8_t q[9], cell[32], clean[32];
    assert(input && fseek(input,(long)index*9,SEEK_SET)==0 && fread(q,1,9,input)==9);
    assert(fclose(input)==0);
    assert(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0) && original_sound_init(clean,ORIGINAL_SOUND_SYNTHONE,0));
    static const unsigned ids[9]={ORIGINAL_SYNTHONE_LFO1_SHAPE,ORIGINAL_SYNTHONE_LFO1_LEVEL,ORIGINAL_SYNTHONE_LFO1_RATE,
        ORIGINAL_SYNTHONE_LFO2_SHAPE,ORIGINAL_SYNTHONE_LFO2_LEVEL,ORIGINAL_SYNTHONE_LFO2_RATE,ORIGINAL_SYNTHONE_CRUSH,
        ORIGINAL_SYNTHONE_CRUSH_LFO,ORIGINAL_SYNTHONE_TREM};
    for(unsigned i=0;i<9;++i) assert(original_sound_set(cell,ids[i],q[i]));
    assert(original_runtime_init(&dry,clean,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&wet,cell,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    for(unsigned j=0;j<2;++j) {
        sp_phasor_init(sp,phasors+j,0);
        sp_port_init(sp,ports+j,.0053125f);
        unsigned i=3*j;
        parameters[i]=(float)((q[i]*3u+63u)/127u);
        parameters[i+1]=(float)q[i+1]/127.f;
        parameters[i+2]=s1_lfo_hz(q[i+2]);
    }
    parameters[bitCrushSampleRate]=s1_part_crush_hz((float)q[6]/127.f);
    parameters[bitcrushLFO]=(float)((q[7]*3u+63u)/127u);
    parameters[tremoloLFO]=(float)((q[8]*3u+63u)/127u);
    if (argc==5) {
        FILE *authored=fopen(argv[4],"rb");
        assert(authored && fseek(authored,(long)index*9*sizeof(float),SEEK_SET)==0);
        assert(fread(parameters,sizeof(float),9,authored)==9 && fclose(authored)==0);
    }
    assert(wet.dsp.synthone.crush_route==(unsigned)parameters[bitcrushLFO] &&
           wet.dsp.synthone.tremolo==(unsigned)parameters[tremoloLFO]);
    FILE *output=fopen(argv[3],"wb");
    assert(output);
    for(unsigned at=0;at<44100;) {
        if (at==0) note(54,100);
        if (at==11025) note(61,93);
        if (at==22050) { note(54,0); note(61,0); note(66,116); }
        if (at==33075) note(66,0);
        unsigned n=11025-at%11025;
        if(n>128)n=128;
        float dl[128],dr[128],wl[128],wr[128],result[256];
        assert(original_runtime_render(&dry,&scratch,dl,dr,n)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_render(&wet,&scratch,wl,wr,n)==ORIGINAL_RUNTIME_OK);
        for(unsigned i=0;i<n;++i) {
            assert(dl[i]==dr[i] && wl[i]==wr[i]);
            result[i*2]=original_stage(dl[i]); result[i*2+1]=wl[i];
        }
        assert(fwrite(result,sizeof(float),2*n,output)==2*n);
        at+=n;
    }
    assert(fclose(output)==0);
}
