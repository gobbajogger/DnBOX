# Native SuperJ6 Sub tuning with the original profile actively installed

This is a raw isolated PI32v2 executable run in the strict emulator at 360 MHz, not a firmware image or untouched original AU playback. It compiles the actual compact Sub renderer and the immutable profile inlet from sub-keyclock-fit.md. The original app clock table is provided as input on this local bench only, converted to the established 388 B milli-cent wire format. No proprietary table, original app binary, sample or render is included.

Key points: the real installer accepts the pinned 95-row profile before any typed owner starts. The Sub voice phase step changes by nominal minus loaded **0 / +0.01822281 / −0.02027130 Hz** for physical keys **32 / 36 / 55**. This agrees with the original key-36 sampler bench (+0.018224 Hz nominal model) and held-out key 55 (opposite tuning sign). Incoming key 48 in Low range resolves to physical key 36 with the same phase step. All seven scenarios rendered nonzero audio and matched the expected phase; counters reported zero mismatches. Installing again after initialization was rejected. This only verifies Sub pitch behavior, not signed whole-engine output parity, absolute dry gain, or firmware resource fit with the profile installed.

To reproduce, create a private header declaring `static const dnbx_sj6_sub_tune_profile original_profile` from the locally authorized, SHA-pinned original as in sub-keyclock.md: signed milli-cents are negative 1000 times each source cents value, rounded to integer and stored little-endian in 24 bits; keys absent from the reached table are zero. Do not copy the header into Git. Compile with the exact shared-inlet donor, PI32v2 R3 FPU, native typed test (filter velocity is zero in this fixture), SJ6 init/noise/OTA/profile enabled, and no contraction. Reuse the ladder raw payload start.S/bench.ld; prepend 0x120 bytes to the .text output, boot in the emulator in strict/no-skip mode, and inspect the RAM report at offset 0x10000 (four little-endian u32 then seven tuples of three little-endian floats). The harness clears private process state between its two arms, not a production feature.

### bench.c

```c
#include <stdint.h>
#include "dnbx_core.c"
#include "profile.private.h"
#include "fm1_time.h"

static dnbx_part s;
static float l[64],r[64];
static unsigned failures;
static unsigned scenario(unsigned note,unsigned physical,float range,volatile float *res)
{
    if(dnbx_init_superj6(&s,44100.f)!=DNBX_OK)return 1;
    const uint8_t codes[2]={0,127};
    dnbx_set_sj6_sources(&s,0,codes);
    if(dnbx_store_raw(&s,86,range)!=DNBX_RAW_STORED_ONLY)return 1;
    int v=dnbx_note_on(&s,note,86);
    if(v<0||v>=DNBX_VOICES)return 1;
    float before=s.voice[v].sj6_sub_phase;
    if(dnbx_render_pitch_osc_levels_velocity(&s,l,r,1,1.f,.5f,.3f,0.f)!=DNBX_OK)return 1;
    float after=s.voice[v].sj6_sub_phase;
    float delta=after-before;if(delta<0.f)delta+=1.f;
    int mc=sj6_profile?sj6_sub_tune_mc(sj6_profile,physical):0;
    float expect=.5f*pitch_increment(&s,physical,(float)mc*.00001f);
    float exp_phase=before+expect;if(exp_phase>=1.f)exp_phase-=1.f;
    res[0]=delta*44100.f;res[1]=expect*44100.f;
    float peak=0.f;
    for(unsigned i=0;i<16;++i){
      if(dnbx_render_pitch_osc_levels_velocity(&s,l,r,64,1.f,.5f,.3f,0.f)!=DNBX_OK)return 1;
      for(unsigned j=0;j<64;j++)if(fm_fabsf(l[j])>peak)peak=fm_fabsf(l[j]);
    }
    res[2]=peak;
    return after!=exp_phase||peak==0.f;
}
void bench_start(void){
    volatile uint32_t *out=(void*)0x01c10000;
    volatile float *m=(volatile float*)(out+4);
    unsigned fail=0;out[0]=0;fm1_time_init();
    const unsigned k[]={32,36,55}; uint32_t t=fm1_ticks();
    /* Isolated program reset; production API still has no reset or rebind path. */
    sj6_profile=0;sj6_profile_closed=0;
    for(unsigned i=0;i<3;++i)fail+=scenario(k[i],k[i],.5f,m+3*i);
    sj6_profile=0;sj6_profile_closed=0;
    fail+=!dnbx_sj6_install_sub_tune_profile(&original_profile,sizeof original_profile);
    for(unsigned i=0;i<3;++i)fail+=scenario(k[i],k[i],.5f,m+9+3*i);
    fail+=scenario(48,36,0.f,m+18);
    fail+=dnbx_sj6_install_sub_tune_profile(&original_profile,sizeof original_profile);
    out[1]=fail;out[2]=fm1_ticks()-t;out[3]=sizeof original_profile;out[0]=0x534a3654;
}
void *memset(void *p,int v,size_t n){volatile unsigned char *d=p;while(n--)*d++=(unsigned char)v;return p;}
void *memcpy(void *p,const void *s,size_t n){volatile unsigned char *d=p;const unsigned char *a=s;while(n--)*d++=*a++;return p;}
```

### result.json

```json
{
  "magic": "0x534a3654",
  "fail": 0,
  "ticks": 1149160,
  "profile_bytes": 388,
  "phase_delta_hz": [
    0.0,
    0.018222808837890625,
    -0.02027130126953125
  ],
  "cases": [
    [
      25.95654296875,
      25.95654296875,
      0.05124349892139435
    ],
    [
      32.70319366455078,
      32.70319366455078,
      0.05138986557722092
    ],
    [
      97.99886322021484,
      97.99886322021484,
      0.044937457889318466
    ],
    [
      25.95654296875,
      25.95654296875,
      0.05124349892139435
    ],
    [
      32.68497085571289,
      32.68497085571289,
      0.05142862722277641
    ],
    [
      98.01913452148438,
      98.01913452148438,
      0.0448896661400795
    ],
    [
      32.68497085571289,
      32.68497085571289,
      0.06591219455003738
    ]
  ]
}
```
