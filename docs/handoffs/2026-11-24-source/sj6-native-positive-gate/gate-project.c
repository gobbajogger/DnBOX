#define SMC_UI_MAIN kept_fixture_main
#include "smc_ui_test.c"
int main(int argc,char **argv)
{
    CHECK(argc==2);
    setup(V_SEQ,PART_303A);
    proj.set.bt |= PROJ_BT_CLK_IN;
    const unsigned kind[]={ORIGINAL_SOUND_JUNO,ORIGINAL_SOUND_JUNO};
    for(unsigned v=0;v<2;++v){
        CHECK(engine_bass_select_preset(v,kind[v],0,&proj.sound)==0);
        for(int n=0;n<1024&&engine_bass_preset_pending(v);++n){
            int32_t buf[512];engine_render(buf,256);
        }
        CHECK(!engine_bass_preset_pending(v) && engine_bass_bank(v)==(int)kind[v]);
        engine_arp_cfg c; engine_arp_cfg_default(&c); c.on=1; c.range=2; c.gate=50; c.division=15; seq.ch[TRK_BASS0+v]=(uint8_t)(1+v);
        engine_arp_block_encode(&c,(uint8_t *)proj.sound.v[T_303][ARP_BLOCK_VOICE0+v]);
    }
    for(unsigned v=DR_CH;v<=DR_RD;++v) {
        unsigned i=0;
        for(;i<drum909_nparams(v);++i)
            if(!strcmp(drum909_param(v,i)->name,"Tune"))break;
        CHECK(i<drum909_nparams(v));
        engine_set(T_909,v,i,127);
        proj.sound.v[T_909][v][i]=127;
        for(unsigned t=0;t<16;t+=2)sm_set(proj.pat[0].drum[0].hit[v],t);
    }
    for(unsigned v=0;v<2;++v)for(unsigned t=0;t<16;t++){
        const bstep_t step={(uint8_t)(48+(t+v*4)%12),G_NOTE|BS_ACCENT,110};
        proj.pat[0].bass[v].step[t]=step;
    }
    CHECK(engine_bass_sound_validate(&proj.sound)==0);
    CHECK(project_validate_for(&proj)==0);
    CHECK(project_save()==0);
    FILE *f=fopen(argv[1],"wb");CHECK(f!=NULL);
    for(uint32_t j=0;j<OBJ_NOBJ;j++){
        if(store_len[j]<0)continue;
        uint32_t n=(uint32_t)store_len[j];
        CHECK(fwrite(&j,4,1,f)==1&&fwrite(&n,4,1,f)==1&&fwrite(store[j],1,n,f)==n);
    }
    CHECK(!fclose(f));
    printf("sound bytes=%zu project sound offset=%zu kinds=%u/%u arp both; 909 CH/OH/CR/RD max Tune\n",sizeof(proj.sound),offsetof(project_t,sound),kind[0],kind[1]);
    return 0;
}
