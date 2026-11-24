/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1-filter-env-authored-helpers.h"
int main(void)
{
    const char *path=getenv("S1_EDITOR_TEST_PACK"); CHECK(path);
    FILE *file=fopen(path,"rb"); CHECK(file);
    unsigned bytes=(unsigned)fread(pack,1,sizeof pack,file);
    CHECK(fgetc(file)==EOF && !fclose(file) && s1_part_set_pack(pack,bytes)==694);
    /* Original pack row 4 has a literal LFO2 route and enabled noise. */
    static s1_dry_part factory;
    CHECK(s1_part_program(&factory,3) && factory.e.t[56]==2.f && factory.e.t[12]>.1f);
    for(unsigned owner=0;owner<NBASS;++owner) {
        setup(V_PART,PART_303A+owner); memset(proj.arr.lane,0,sizeof proj.arr.lane);
        proj.set.bt|=PROJ_BT_CLK_IN;
        choose(owner,4); choose(1-owner,0); visit("FILTER MODE");
        CHECK(read_slot(owner,56).position==2 && read_slot(owner,56).physical==2.f);
        CHECK(!proj.sound.v[T_303][owner][30]);
        detent(3,1);
        CHECK(read_slot(owner,56).position==3 && read_slot(owner,56).physical==3.f);
        CHECK(proj.sound.v[T_303][owner][30]==100u);
        stabilize(); CHECK(!project_save() && !project_load());
        CHECK(read_slot(owner,56).physical==3.f);
        capture_lock(owner); choose(owner,0); choose(owner,4);
        CHECK(!proj.sound.v[T_303][owner][30]);
        CHECK(read_slot(owner,56).physical==2.f && host_s1_owner(owner)->t[56]==2.f);
        lock_roundtrip(owner,3.f,2.f);
    }
    printf("PASS real factory row 4 LFO2 word, adjacent BOTH, persistence and held-temporary route restored exact on both owners (%u checks)\n", checks);
    return 0;
}
