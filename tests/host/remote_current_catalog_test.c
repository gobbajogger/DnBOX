/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual current UI/engine descriptor census, not an admitted DNR2 catalog.
 * No protocol IDs, writable transport or hardware endpoint are introduced. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned rows, edits;
static void census(unsigned t, unsigned v, int bank, FILE *out)
{
    unsigned n=(unsigned)engine_nparams(t,v);
    CHECK(n<=NPARAMS_MAX);
    for(unsigned i=0;i<n;i++) {
        if(t==T_303 && original_sound_is_performance((unsigned)bank,i)) continue;
        pref_t ref=PR(R_ENG,t,v,i);
        const x0x_param_t *d=pref_desc(ref);
        CHECK(d && d->name && *d->name); CHECK(!pref_read_only(ref));
        unsigned max=d->max;
        /* Only test values admitted by the current descriptor, with a real
         * audio drain between operations. This is not queue-race coverage. */
        const unsigned values[]={0,max/2,max};
        for(unsigned k=0;k<3;k++) {
            pref_set_impl(ref,(int)values[k],0);
            CHECK(pref_get(ref)==(int)values[k]);
            if(t==T_303)CHECK(engine_bass_sound_validate(&proj.sound)==0);
            run_ms(20); edits++;
        }
        fprintf(out,"%s{\"target\":%u,\"voice\":%u,\"parameter\":%u,\"bank\":%d,\"minimum\":0,\"maximum\":%u}", rows?",\n":"",t,v,i,bank,max);
        rows++;
    }
}
int main(void)
{
    setup(V_HOME,PART_303A);
    const char *path=getenv("DNBOX_CURRENT_CATALOG_OUT");
    CHECK(path && *path); FILE *out=fopen(path,"w"); CHECK(out);
    fputs("{\"schema\":\"dnbox-current-host-descriptor-census/1\",\"admitted\":false,\"protocolIdsAssigned\":false,\"controls\":[\n",out);
    for(unsigned t=0;t<NTARGETS;t++) {
        if(t==T_303)continue;
        for(unsigned v=0;v<(unsigned)engine_nvoices(t);v++)census(t,v,-1,out);
    }
    for(unsigned bank=ORIGINAL_SOUND_CHICAGO;bank<=ORIGINAL_SOUND_MIAMI;bank++) {
        for(unsigned v=0;v<NBASS;v++) {
            CHECK(engine_bass_select_preset(v,bank,0,&proj.sound)==0);
            run_ms(40); CHECK(!engine_bass_preset_pending(v));
            CHECK(engine_bass_bank(v)==(int)bank);
            census(T_303,v,(int)bank,out);
        }
    }
    fprintf(out,"\n],\"count\":%u,\"validatedScalarEdits\":%u}\n",rows,edits);
    CHECK(fclose(out)==0);
    printf("PASS current host descriptor census: %u rows, %u scalar edits, %u checks; NOT a protocol admission\n",rows,edits,checks);
    return 0;
}
