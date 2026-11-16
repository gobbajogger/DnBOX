/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void rejected(unsigned t,unsigned v,unsigned p,int bank,unsigned value)
{
    ui.dirty=0;
    sound_t before=proj.sound;unsigned dirty=ui.dirty;
    unsigned available=engine_queue_available_locked();
    CHECK(dnbox_remote_scalar_set(t,v,p,bank,value)==0);
    CHECK(!memcmp(&before,&proj.sound,sizeof before));CHECK(ui.dirty==dirty);CHECK(engine_queue_available_locked()==available);
}
extern void engine_host_cq_at(uint32_t cursor);
static void empty_queue(void)
{
    /* CQ is 1024 entries in the current engine source. Do not confuse one
     * audio boundary (at most128 commands) with complete queue retirement. */
    for(unsigned n=0;n<100 && engine_queue_available_locked()!=1024;n++)run_ms(10);
    CHECK(engine_queue_available_locked()==1024);
}
int main(void)
{
    setup(V_HOME,PART_303A);empty_queue();
    engine_host_cq_at(UINT32_MAX-4u);
    CHECK(engine_queue_available_locked()==1024);
    for(unsigned n=0;n<8;n++)engine_set(T_MIX,0,MX_LEVEL,n);
    CHECK(engine_queue_available_locked()==1016);empty_queue();
    rejected(NTARGETS,0,0,-1,0);rejected(T_MST,1,0,-1,0);
    rejected(T_MST,0,NPARAMS_MAX,-1,0);rejected(T_MST,0,MST_MAKEUP,0,0);
    rejected(T_MST,0,MST_MAKEUP,-1,UINT32_MAX);
    project_busy=1;rejected(T_MST,0,MST_MAKEUP,-1,0);project_busy=0;
    CHECK(dnbox_remote_scalar_set(T_MST,0,MST_MAKEUP,-1,0)==2);run_ms(20);
    CHECK(engine_master_get(MST_MAKEUP)==0);
    for(unsigned i=0;i<2048;i++)engine_set(T_MIX,0,MX_LEVEL,i&127u);
    rejected(T_MST,0,MST_MAKEUP,-1,127);empty_queue();
    CHECK(engine_master_get(MST_MAKEUP)==0);
    CHECK(dnbox_remote_scalar_set(T_MST,0,MST_MAKEUP,-1,127)==2);
    CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==127);run_ms(20);
    CHECK(engine_master_get(MST_MAKEUP)==127);
    empty_queue();
    for(unsigned n=0;n<1023;n++)engine_set(T_MIX,0,MX_LEVEL,n&127u);
    CHECK(engine_queue_available_locked()==1);
    CHECK(dnbox_remote_scalar_set(T_MST,0,MST_MAKEUP,-1,64)==2);
    CHECK(engine_queue_available_locked()==0);
    rejected(T_MST,0,MST_MAKEUP,-1,65);empty_queue();
    CHECK(engine_master_get(MST_MAKEUP)==64);
    uint8_t th,ratio,makeup,mode,cutoff;
    CHECK(dnbox_remote_scalar_set(T_MST,0,MST_COMP1,-1,73)==2);
    master_comp1_pots(73,&th,&ratio,&makeup);
    CHECK(proj.sound.v[T_MST][0][MST_THRESH]==th);
    CHECK(proj.sound.v[T_MST][0][MST_RATIO]==ratio);
    CHECK(proj.sound.v[T_MST][0][MST_MAKEUP]==makeup);empty_queue();
    CHECK(engine_master_get(MST_THRESH)==th);CHECK(engine_master_get(MST_MAKEUP)==makeup);
    CHECK(dnbox_remote_scalar_set(T_MST,0,MST_DJF,-1,33)==2);
    master_djf_pots(33,&mode,&cutoff);
    CHECK(proj.sound.v[T_MST][0][MST_MODE]==mode);
    CHECK(proj.sound.v[T_MST][0][MST_CUTOFF]==cutoff);empty_queue();
    CHECK(engine_master_get(MST_MODE)==mode);CHECK(engine_master_get(MST_CUTOFF)==cutoff);
    sound_t saved=proj.sound;
    proj.sound.bass[0].bytes[0]^=1;
    rejected(T_303,0,0,engine_bass_bank(0),0);proj.sound=saved;
    proj.sound.v[T_303][0][29]=255;
    rejected(T_303,0,0,engine_bass_bank(0),0);proj.sound=saved;
    CHECK(original_sound_init(proj.sound.v[T_303][0],(unsigned)(1-engine_bass_bank(0)),0));
    engine_bass_sound_refresh(&proj.sound,0);CHECK(engine_bass_sound_validate(&proj.sound)==0);
    rejected(T_303,0,0,engine_bass_sound_bank(&proj.sound,0),0);proj.sound=saved;
    unsigned edits=0;
    for(unsigned t=0;t<NTARGETS;t++)for(unsigned v=0;v<(unsigned)engine_nvoices(t);v++) {
        if(t==T_303)continue;
        for(unsigned p=0;p<(unsigned)engine_nparams(t,v);p++) {
            const x0x_param_t*d=engine_param(t,v,p);CHECK(d);
            rejected(t,v,p,-1,(unsigned)d->max+1);
            unsigned values[]={0,d->max/2,d->max};
            for(unsigned i=0;i<3;i++){CHECK(dnbox_remote_scalar_set(t,v,p,-1,values[i])==2);CHECK(pref_get(PR(R_ENG,t,v,p))==(int)values[i]);run_ms(20);edits++;}
        }
    }
    for(unsigned bank=0;bank<2;bank++)for(unsigned v=0;v<NBASS;v++) {
        CHECK(engine_bass_select_preset(v,bank,0,&proj.sound)==0);
        CHECK(engine_bass_preset_pending(v));
        rejected(T_303,v,0,(int)bank,0);
        rejected(T_303,v,0,engine_bass_bank(v),0);
        run_ms(40);CHECK(!engine_bass_preset_pending(v));
        rejected(T_303,v,0,(int)(1-bank),0);
        for(unsigned p=0;p<original_sound_count(bank);p++) {
            if(original_sound_is_performance(bank,p)){rejected(T_303,v,p,(int)bank,0);continue;}
            const x0x_param_t*d=original_sound_param(bank,p);CHECK(d);
            rejected(T_303,v,p,(int)bank,(unsigned)d->max+1);
            unsigned values[]={0,d->max/2,d->max};
            for(unsigned i=0;i<3;i++){CHECK(dnbox_remote_scalar_set(T_303,v,p,(int)bank,values[i])==2);CHECK(pref_get(PR(R_ENG,T_303,v,p))==(int)values[i]);CHECK(engine_bass_sound_validate(&proj.sound)==0);run_ms(20);edits++;}
        }
    }
    printf("PASS checked current scalar transaction: %u edits, %u checks; no transport or revision/held-owner qualification\n",edits,checks);
    return 0;
}
