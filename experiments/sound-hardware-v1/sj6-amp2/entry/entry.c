/* Full app path, using the existing UI/project/engine fixture. */
#define SMC_UI_MAIN original_smc_fixture_main
#include "smc_ui_test.c"

static void drain(void)
{
    int32_t audio[512];
    engine_render(audio,256);
}
int main(void)
{
    const char *mode=getenv("SJ6_ENTRY_CASE");
    const char *out=getenv("SJ6_ENTRY_PCM");
    CHECK(mode && out);
    setup(V_PART,PART_303A);
    CHECK(original_sound_count(ORIGINAL_SOUND_JUNO)==18);
    CHECK(original_sound_factories(ORIGINAL_SOUND_JUNO)==4);
    int legal[4]={1,1,1,1};
    for(unsigned f=0;f<4;++f)
        legal[f]=original_sound_name(ORIGINAL_SOUND_JUNO,f)!=NULL;
    CHECK(legal[0] && legal[1] && legal[2] && legal[3]);
    CHECK(engine_bass_select_preset(0,ORIGINAL_SOUND_JUNO,4,&proj.sound)!=0);
    for(unsigned owner=0;owner<2;++owner) {
        CHECK(engine_bass_select_preset(owner,ORIGINAL_SOUND_JUNO,owner,&proj.sound)==0);
        for(unsigned n=0;n<1024 && engine_bass_preset_pending(owner);++n) drain();
        CHECK(!engine_bass_preset_pending(owner) && engine_bass_bank(owner)==ORIGINAL_SOUND_JUNO);
        CHECK(engine_bass_preset_index(owner)==(int)owner);
        CHECK(engine_bass_sound_bank(&proj.sound,owner)==ORIGINAL_SOUND_JUNO);
    }
    if(!strcmp(mode,"stock-attack"))
        engine_set(T_303,0,5,127);
    else if(!strcmp(mode,"tagged-amp2")) {
        for(unsigned id=119;id<=122;++id) engine_set(T_303,0,256+id,127);
    } else CHECK(!strcmp(mode,"unedited"));
    drain();
    engine_bass_on_velocity(0,60,0,0,105);
    engine_bass_on_velocity(1,67,0,0,92);
    FILE *f=fopen(out,"wb");
    CHECK(f);
    unsigned nonzero=0;
    for(unsigned block=0;block<128;++block) {
        if(block==64) engine_bass_off(0);
        if(block==96) engine_bass_off(1);
        int32_t audio[512];
        engine_render(audio,256);
        for(unsigned i=0;i<512;++i) nonzero+=audio[i]!=0;
        CHECK(fwrite(audio,sizeof *audio,512,f)==512);
    }
    CHECK(fclose(f)==0 && nonzero>1000);
    printf("case=%s stereo_frames=32768 nonzero_words=%u\n",mode,nonzero);
}
