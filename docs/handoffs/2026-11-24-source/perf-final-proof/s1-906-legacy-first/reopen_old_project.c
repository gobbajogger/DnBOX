/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"
#include "../../firmware/src/dsp/s1pack/s1pack.h"
static uint8_t old[100000],combined[100000],ord[4];
static char names[3][48];
static int32_t before[3][NBASS][FRAMES*2];
static unsigned read_file(const char *var,uint8_t *dst,unsigned lim)
{
    const char *file=getenv(var); CHECK(file);
    FILE *f=fopen(file,"rb"); CHECK(f);
    unsigned n=(unsigned)fread(dst,1,lim,f);
    CHECK(fgetc(f)==EOF && !fclose(f));
    return n;
}
static void choose(unsigned owner,unsigned index)
{
    CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_SYNTHONE,index,&proj.sound));
    for(unsigned n=0;n<1024 && engine_bass_preset_pending(owner);++n) render(1);
    CHECK(!engine_bass_preset_pending(owner) && engine_bass_preset_index(owner)==(int)index);
}
int main(void)
{
    unsigned na=read_file("OLD_PACK",old,sizeof old),nb=read_file("COMBINED_PACK",combined,sizeof combined);
    CHECK(read_file("OLD_ALIAS_PAIR",ord,sizeof ord)==sizeof ord);
    const unsigned ids[]={44u,(unsigned)ord[0]|((unsigned)ord[1]<<8),(unsigned)ord[2]|((unsigned)ord[3]<<8)};
    CHECK(ids[1]!=ids[2] && ids[1]!=44u && ids[2]!=44u);
    CHECK(s1_part_set_pack(old,na)==694);
    for(unsigned i=0;i<3;++i) {
        snprintf(names[i],sizeof names[i],"%s",original_sound_name(ORIGINAL_SOUND_SYNTHONE,ids[i]));
        CHECK(names[i][0]);
    }
    CHECK(strcmp(names[1],names[2]));
    for(unsigned i=0;i<3;++i) for(unsigned owner=0;owner<NBASS;++owner) {
        CHECK(s1_part_set_pack(old,na)==694);
        setup(V_PART,PART_303A+owner); proj.set.bt|=PROJ_BT_CLK_IN;
        choose(owner,ids[i]); choose(1-owner,0);
        CHECK(engine_bass_preset_index(owner)==(int)ids[i]);
        stabilize(); CHECK(!project_save());
        stored_project("legacy-reopen",owner,PROJECT_EXPORT);
        begin_pair(owner,57); record(before[i][owner]);
        CHECK(s1_part_set_pack(combined,nb)==907);
        stored_project("legacy-reopen",owner,PROJECT_MOUNT);
        const unsigned writes=store_writes;
        CHECK(!project_load() && !project_storage_unsupported());
        CHECK(engine_bass_preset_index(owner)==(int)ids[i]);
        CHECK(!strcmp(names[i],original_sound_name(ORIGINAL_SOUND_SYNTHONE,ids[i])));
        CHECK(store_writes==writes);
        stored_project("legacy-reopen",owner,PROJECT_VERIFY);
        begin_pair(owner,57); record(changed);
        CHECK(!memcmp(before[i][owner],changed,sizeof changed));
    }
    for(unsigned owner=0;owner<NBASS;++owner)
        CHECK(!memcmp(before[1][owner],before[2][owner],sizeof before[1][owner]));
    printf("PASS authentic previous-format old project ordinal 44 plus distinct-label same-content pair; both owners; full saved bytes no migration and exact complete compact mix PCM (%u checks)\n",checks);
    return 0;
}
