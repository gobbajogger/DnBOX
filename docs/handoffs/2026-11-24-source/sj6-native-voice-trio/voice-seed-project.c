#include "gate-seed.inc"
int main(int argc,char **argv)
{
    CHECK(argc==3);
    if(!strcmp(argv[1],"gate")){argv[1]=argv[2];return gate_seed_main(2,argv);}
    CHECK(!strcmp(argv[1],"voice"));
    setup(V_PART,PART_303A);
    project_reset_stage();
    project_stage.set.snd_rev=REV137;
    project_stage.set.bt|=PROJ_BT_CLK_IN;
    for(unsigned v=0;v<NBASS;v++){
        CHECK(original_sound_init(project_stage.sound.v[T_303][v],ORIGINAL_SOUND_JUNO,0));
        engine_bass_sound_refresh(&project_stage.sound,v);
        slot137(&project_stage.sound,v)[0]=137u;
        slot137(&project_stage.sound,v)[1]=(uint8_t)v;
        uint8_t *voice=sj6_voice_payload(&project_stage.sound,v);
        voice[0]=sj6_voice_tag(v,0,63u+v*9u);voice[1]=(uint8_t)(63u+v*9u);
        engine_arp_cfg c;engine_arp_cfg_default(&c);
        c.on=0;c.gate=50;c.division=15;
        engine_arp_block_encode(&c,project_stage.sound.v[T_303][ARP_BLOCK_VOICE0+v]);
    }
    CHECK(!project_validate_stage());project_commit_stage();
    seq.ch[TRK_BASS0]=1;seq.ch[TRK_BASS0+1]=2;
    CHECK(!project_save());
    FILE *f=fopen(argv[2],"wb");CHECK(f);
    for(uint32_t j=0;j<OBJ_NOBJ;j++){
      if(store_len[j]<0)continue;
      uint32_t n=(uint32_t)store_len[j];
      CHECK(fwrite(&j,4,1,f)==1&&fwrite(&n,4,1,f)==1&&fwrite(store[j],1,n,f)==n);
    }
    CHECK(!fclose(f));
}
