/* SPDX-License-Identifier: GPL-3.0-only */
/* A project saved by the removed bass303 engine (sound rev 10, streams 3
 * and 6) must load safely in the Chicago/Miami build: no crash, the part
 * falls back to the Chicago power-on sound, and the old file stays on the
 * store untouched (no implicit save). Also exercise legacy ODR1 owner migration. */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include "app/original_runtime.h"
static unsigned checks;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"FAIL legacy-303 line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static uint8_t legacy[65536];
static sound_t chicago_default;

static size_t read_fixture(const char *path)
{
    FILE *f=fopen(path,"rb"); CHECK(f!=0);
    size_t n=fread(legacy,1,sizeof legacy,f); fclose(f);
    CHECK(n>32+sizeof(settings_t)); return n;
}
static void check_chicago_default(void)
{
    CHECK(!memcmp(proj.sound.v[T_303],chicago_default.v[T_303],sizeof proj.sound.v[T_303]));
    engine_init(proj.pat,&proj.arr.song,proj.arr.lane,&proj.sound);
    engine_apply_sound(&proj.sound);
    int32_t audio[512]; for(int i=0;i<64;i++) engine_render(audio,256);   /* renders; no crash */
}
static void file_import(size_t n)
{
    /* Editor/web import: the inner file is rejected, the stage is not adopted. */
    uint8_t h[32]; memcpy(h,legacy,32);
    size_t body=n-32; if(body>sizeof project_stage) body=sizeof project_stage;
    memset(&project_stage,0,sizeof project_stage); memcpy(&project_stage,legacy+32,body);
    CHECK(project_file_validate(h)!=0);
    project_defaults(); check_chicago_default();
}
static void flash_load(void)
{
    /* Device boot: old settings/sound objects on the store. */
    uint32_t fmt=get32(legacy+16);
    uint8_t buf[4+sizeof(settings_t)+sizeof(sound_t)];
    put32(buf,fmt); memcpy(buf+4,legacy+32,sizeof(settings_t));
    CHECK(!plat_store_save(OBJ_SET,buf,4+sizeof(settings_t)));
    put32(buf,fmt); memcpy(buf+4,legacy+32+sizeof(settings_t),sizeof(sound_t)-64);
    CHECK(!plat_store_save(OBJ_SOUND,buf,4+sizeof(sound_t)-64));
    unsigned writes=store_writes;
    CHECK(project_load()!=0);
    CHECK(project_storage_unsupported());
    check_chicago_default();
    CHECK(store_writes==writes);                                   /* old file kept */
}
/* Install a genuine historical wire identity in a normal seeded project.
 * The old knob bytes are intentionally not a valid modern INIT cell. */
static void flash_load_legacy_slot2(unsigned owner)
{
    const unsigned peer=owner^1u;
    static const uint8_t cathedral[6]={57,66,51,23,83,95};
    uint8_t raw[4+sizeof(sound_t)], reread[sizeof raw], initial[ORIGINAL_SOUND_BYTES];
    sound_t legacy;
    boot();
    CHECK(project_save()==0);
    int n=plat_store_load(OBJ_SOUND,raw,sizeof raw);
    CHECK(n==sizeof raw);
    memcpy(&legacy,raw+4,sizeof legacy);
    uint8_t *old=legacy.v[T_303][owner];
    memset(old,0,ORIGINAL_SOUND_BYTES);
    memcpy(old,cathedral,sizeof cathedral);
    old[26]='O'; old[27]='S'; old[28]=ORIGINAL_SOUND_SCHEMA;
    old[29]=2; old[30]=3;
    uint8_t *tag=legacy.bass[owner].bytes;
    memset(tag,0,sizeof legacy.bass[owner].bytes);
    put32(tag,0x4f445231u); /* Historical ODR1, not the modern generated ID. */
    tag[4]=ORIGINAL_SOUND_SCHEMA; tag[6]=8; tag[7]=BASS_IDENTITY_KNOWN; tag[8]=3;
    CHECK(original_sound_is_legacy_slot2_cell(old));
    memcpy(raw+4,&legacy,sizeof legacy);
    CHECK(plat_store_save(OBJ_SOUND,raw,n)==0);
    const unsigned writes=store_writes;
    memset(&proj.sound,0,sizeof proj.sound);
    CHECK(project_load()==0 && !project_storage_unsupported());
    CHECK(original_sound_init(initial,ORIGINAL_SOUND_SYNTHONE,ORIGINAL_SYNTHONE_INIT_FACTORY));
    CHECK(!memcmp(initial,proj.sound.v[T_303][owner],sizeof initial));
    CHECK(!memcmp(legacy.v[T_303][peer],proj.sound.v[T_303][peer],sizeof initial));
    CHECK(!memcmp(&legacy.bass[peer],&proj.sound.bass[peer],sizeof legacy.bass[peer]));
    CHECK(get32(proj.sound.bass[owner].bytes)==ORIGINAL_SOUND_SYNTHONE_ID);
    CHECK(proj.sound.bass[owner].bytes[8]==ORIGINAL_SYNTHONE_INIT_FACTORY);
    CHECK(store_writes==writes &&
          plat_store_load(OBJ_SOUND,reread,sizeof reread)==n && !memcmp(raw,reread,n));

    engine_init(proj.pat,&proj.arr.song,proj.arr.lane,&proj.sound);
    engine_apply_sound(&proj.sound);
    CHECK(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE);
    CHECK(engine_bass_preset_index(owner)==ORIGINAL_SYNTHONE_INIT_FACTORY);
    CHECK(engine_bass_status(owner)==ORIGINAL_RUNTIME_OK);
    CHECK(engine_bass_bank(peer)==engine_bass_sound_bank(&legacy,peer));
    int32_t out[512];
    engine_render(out,256);

    /* An explicit save, unlike load, persists the new identity, then reloads. */
    CHECK(project_save()==0);
    CHECK(plat_store_load(OBJ_SOUND,reread,sizeof reread)==n);
    sound_t saved;
    memcpy(&saved,reread+4,sizeof saved);
    CHECK(get32(saved.bass[owner].bytes)==ORIGINAL_SOUND_SYNTHONE_ID);
    CHECK(!memcmp(saved.v[T_303][owner],initial,sizeof initial));
    CHECK(!memcmp(saved.v[T_303][peer],legacy.v[T_303][peer],sizeof initial));
    CHECK(!memcmp(&saved.bass[peer],&legacy.bass[peer],sizeof legacy.bass[peer]));
    CHECK(project_load()==0 && !memcmp(proj.sound.v[T_303][owner],initial,sizeof initial));
}

int main(int argc,char **argv)
{
    CHECK(argc>=2);
    flash_load_legacy_slot2(0); flash_load_legacy_slot2(1);
    if(argc==2 && !strcmp(argv[1],"--slot2-only")){
        printf("ok legacy ODR1 project: %u checks, both owners migrate to Synth One INIT\n",checks);
        return 0;
    }
    engine_sound_defaults(&chicago_default);
    for(int i=1;i<argc;i++){ size_t n=read_fixture(argv[i]); file_import(n); flash_load(); }
    printf("ok legacy-303-project / ODR1 slot-2: %u checks, %d old bass303 fixtures map to Chicago\n",checks,argc-1);
    return 0;
}
