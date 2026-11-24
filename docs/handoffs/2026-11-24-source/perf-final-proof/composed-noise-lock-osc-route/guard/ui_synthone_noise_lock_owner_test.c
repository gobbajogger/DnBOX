/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
void host_lfo_capture(unsigned o,const float*l,const float*r,unsigned n){(void)o;(void)l;(void)r;(void)n;}

int main(void)
{
    for(unsigned owner=0;owner<NBASS;++owner) {
        setup(V_PART,PART_303A+owner); select_patch(owner);
        proj.set.bt |= PROJ_BT_CLK_IN;
        memset(proj.arr.lane,0,sizeof proj.arr.lane);
#ifdef LEGACY_LOAD
        for(unsigned mode=0;mode<4;mode++) {
            stored_project("noise-lock",4u*owner+mode,PROJECT_MOUNT);
            const uint32_t writes=store_writes;
            CHECK(!project_load() && !project_storage_unsupported());
            CHECK(store_writes==writes && !engine_bass_note_validate(&proj.arr.lane[0]));
            stabilize();
            CHECK(read_slot(owner,53).physical==(float)mode);
            CHECK(read_slot(owner,48).physical==3.f && read_slot(owner,39).physical>80.f);
        }
#else
        CHECK(engine_s1_patch_set(owner,read_slot(owner,39).revision,39,12)==S1_EDIT_QUEUED); render(1);
        CHECK(engine_s1_patch_set(owner,read_slot(owner,48).revision,48,3)==S1_EDIT_QUEUED); render(1);
        const uint8_t *packed=proj.sound.v[T_303][S1_EXTRA_ROW+owner];
        uint8_t saved[15]; memcpy(saved,packed,sizeof saved);
        visit("FM / NOISE");
        /* Store explicit OFF even when the factory is already OFF. */
        CHECK(engine_s1_patch_set(owner,read_slot(owner,53).revision,53,0)==S1_EDIT_QUEUED); render(1);
        for(unsigned mode=0;mode<4;mode++) {
            if(mode) detent(3,1);
            CHECK(read_slot(owner,53).physical==(float)mode);
            CHECK(!memcmp(saved,packed,sizeof saved));
            engine_note_lock_capture(1);
            int result=engine_bass_note_capture(proj.arr.lane,0,owner,0,&proj.sound);
            engine_note_lock_capture(0);
            if(result) fprintf(stderr,"owner %u Noise route %u mixed cutoff/HP capture failed %d\n",owner,mode,result);
            CHECK(!result);
            CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
            CHECK(!memcmp(proj.arr.lane[0].val+BASS_NOTE_USED,packed,sizeof saved));
            CHECK((proj.arr.lane[0].val[25]&7u)==1u+mode);
            stabilize();
            CHECK(!project_save()&&!project_load());
            stored_project("noise-lock",4u*owner+mode,PROJECT_EXPORT);
        }
#endif
    }
#ifdef LEGACY_LOAD
    printf("PASS exact prior reader loads all eight repaired mixed notes before writes (%u checks)\n",checks);
#else
    printf("PASS all eight physical Noise route locks retain same packed cutoff/HP bytes (%u checks)\n",checks);
#endif
    return 0;
}
