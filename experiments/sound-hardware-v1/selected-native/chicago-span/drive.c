#include "engine.h"
#include "original_runtime.h"
#include "x0x.h"
#include "chicago_fx2_compact.h"
extern original_chicago_fx2_compact original_fx[];
extern original_runtime_workspace workspace_b;
extern original_runtime_part original_bass[];
extern original_chicago_tuning original_tuning;
extern original_chicago_fx2_cold_owner original_fx_owner[];
extern volatile unsigned char eng_c1_next;
extern unsigned char eng_c1_mask;
extern uint32_t eng_job_arg;
extern uint16_t original_switch_gain[];
extern float bass_accent_mk[], bass_accent_mk_to[];
#if LEGACY_STAGE
extern uint32_t eng_stage;
extern unsigned char eng_808_split;
#endif
extern void measured(void);
static volatile uint32_t *const result=(void *)0x01c07f00;
static const uint8_t owner=1;

static void job(unsigned frames)
{
    eng_c1_mask=1u<<3;
    eng_c1_next=3;
    eng_job_arg=frames & 255u;
#if LEGACY_STAGE
    eng_808_split=0;
    eng_stage=3;
#endif
    measured();
    ++result[5];
    if (original_bass[owner].status != ORIGINAL_RUNTIME_OK) result[4]=0;
    for(unsigned i=0;i<frames;i++)
        if (workspace_b.engine.channels[0].samples[i]!=0 || workspace_b.engine.channels[1].samples[i]!=0) result[7]=1;
}

static int key(unsigned note, unsigned velocity)
{
    if (original_runtime_prepare(&original_bass[owner],120)!=1) return 0;
    return original_runtime_key(&original_bass[owner],1,note,velocity)==1;
}

void bench(void)
{
    const unsigned factory=result[8], phase=result[9], fx=result[10];
    engine_sound_defaults(&proj.sound);
    result[0]=1;
    engine_init(proj.pat,&proj.arr.song,proj.arr.lane,&proj.sound);
    result[1]=original_bass[owner].status;
    result[2]=original_bass[owner].kind;
    if(result[1]!=1 || result[2]!=0 || original_bass[0].kind!=ORIGINAL_SOUND_MIAMI) return;
    original_switch_gain[owner]=512;
    bass_accent_mk[owner]=bass_accent_mk_to[owner]=1.f;
    if(factory || fx<12) {
        uint8_t cell[32];
        if (!original_sound_init(cell,0,factory)) return;
        if (fx<12) {
            if(!original_sound_set(cell,17,fx) || !original_sound_set(cell,18,104) ||
                !original_sound_set(cell,19,83))return;
        }
        if(original_runtime_retire(&original_bass[owner])!=1)return;
        int rc=original_runtime_init(&original_bass[owner],cell,120,&original_tuning,&original_fx_owner[owner]);
        if(rc!=1){result[1]=rc;return;}
    }
    result[3]=1;
    result[4]=1;
    if(!key(48,110))return;
    job(64);                       /* cold note, aligned control boundary */
    if(phase)job(phase);
    if(!key(55,101))return;
    job(64);                       /* live overlap at each sub-block offset */
    if(!key(55,0))return;
    job(128);                      /* resume the earlier held note */
    if(!key(48,0))return;
    job(256);                      /* release */
    if(!key(60,110))return;
    job(256);                      /* retrigger */
    result[6]=original_fx[owner].selected? original_fx[owner].selected_type : UINT32_MAX;
    result[3]=10+eng_c1_next;
}
