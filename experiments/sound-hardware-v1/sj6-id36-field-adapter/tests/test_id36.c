/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_preset_fields.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define CHECK(predicate) do { if (!(predicate)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #predicate); return 1; \
} } while (0)

static int absolute_attack_cases(void)
{
    static const float controls[] = {0.f, .25f, .5f, .75f, 1.f};
    static const float target_s[] = {.002f,.1601997968f,1.5928740834f,6.1398984597f,16.f};
    for (unsigned k=0; k<sizeof controls/sizeof *controls; ++k) {
        float knob=controls[k];
        sj6_preset_field batch[] = {{185,70.f},{36,knob},{115,0.f},{116,0.f},{119,.9f},{207,0.f},{212,1.f}};
        dnbx_part actual, direct, untouched;
        dnbx_layer_params l1, l2;
        CHECK(dnbx_init(&actual,48000.f)==DNBX_OK);
        CHECK(dnbx_get_layer(&actual,0,&l1)==DNBX_OK);
        CHECK(dnbx_get_layer(&actual,1,&l2)==DNBX_OK);
        l1.pan=-1.f; l2.pan=1.f; l2.amp.attack_s=.071f;
        CHECK(dnbx_set_layer(&actual,0,&l1)==DNBX_OK);
        CHECK(dnbx_set_layer(&actual,1,&l2)==DNBX_OK);
        CHECK(dnbx_store_raw(&actual,12,.7f)==DNBX_RAW_STORED_ONLY);
        direct=actual; untouched=actual;
        int status=sj6_apply_preset_fields(&actual,SJ6_INDEPENDENT_ENVELOPES,batch,sizeof batch/sizeof *batch);
        CHECK(status>=0);
        for(size_t i=0; i<sizeof batch/sizeof *batch; i++)
            CHECK(dnbx_store_raw(&direct,batch[i].id,batch[i].value)>=0);
        CHECK(dnbx_attack_from_unit(knob,&l1.amp.attack_s)==DNBX_OK);
        CHECK(fabsf(l1.amp.attack_s-target_s[k])<1e-5f); /* existing scalar test threshold */
        CHECK(dnbx_set_layer(&direct,0,&l1)==DNBX_OK);
        float got_raw;
        CHECK(dnbx_read_raw(&actual,36,&got_raw)==DNBX_OK && got_raw==knob);
        CHECK(dnbx_get_layer(&actual,0,&l1)==DNBX_OK);
        if (fabsf(l1.amp.attack_s-target_s[k])>=1e-5f) {
            fprintf(stderr,"FAIL ID36 raw %.2f stored but layer 1 attack %.9g s; expected %.9g s\n",knob,l1.amp.attack_s,target_s[k]);
            return 1;
        }
        CHECK(!memcmp(&actual,&direct,sizeof actual));
        CHECK(status==DNBX_OK);
        CHECK(dnbx_note_on(&actual,60,100)==0 && dnbx_note_on(&direct,60,100)==0 && dnbx_note_on(&untouched,60,100)==0);
        float al[512],ar[512],dl[512],dr[512],ul[512],ur[512];
        size_t diff=0, right_nonzero=0;
        for (unsigned block=0; block<4; ++block) {
            CHECK(dnbx_render(&actual,al,ar,512,0)==DNBX_OK);
            CHECK(dnbx_render(&direct,dl,dr,512,0)==DNBX_OK);
            CHECK(dnbx_render(&untouched,ul,ur,512,0)==DNBX_OK);
            CHECK(!memcmp(al,dl,sizeof al) && !memcmp(ar,dr,sizeof ar));
            CHECK(!memcmp(ar,ur,sizeof ar));
            for(size_t i=0; i<512; ++i) {
                CHECK(isfinite(al[i]) && isfinite(ar[i]));
                diff+=al[i]!=ul[i]; right_nonzero+=ar[i]!=0.f;
            }
        }
        CHECK(diff>0 && right_nonzero>0 && !memcmp(&actual,&direct,sizeof actual));
        printf("ID36 raw=%g seconds=%.9g exact_stereo_frames=2048 changed_layer1_samples=%zu unaffected_audible_layer2_samples=%zu\n", knob,l1.amp.attack_s,diff,right_nonzero);
    }
    return 0;
}
static int admission_and_order(void)
{
    dnbx_part p,before,expected;
    CHECK(dnbx_init(&p,48000.f)==DNBX_OK);
    const sj6_preset_field raw[]={{119,.17f},{207,0.f},{212,1.f},{185,70.f}};
    expected=p;
    for(size_t i=0; i<sizeof raw/sizeof *raw; ++i)
        CHECK(dnbx_store_raw(&expected,raw[i].id,raw[i].value)>=0);
    CHECK(sj6_apply_preset_fields(&p,SJ6_LINKS_UNKNOWN,raw,sizeof raw/sizeof *raw)==SJ6_FIELD_STORED);
    CHECK(!memcmp(&expected,&p,sizeof p));
    before=p;
    const sj6_preset_field a={36,.5f};
    CHECK(sj6_apply_preset_fields(&p,SJ6_LINKS_UNKNOWN,&a,1)==SJ6_E_UNQUALIFIED_LINK);
    CHECK(!memcmp(&before,&p,sizeof p));
    sj6_preset_field changes[]={{36,.5f},{115,1.f}};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,changes,2)==SJ6_E_UNQUALIFIED_LINK);
    CHECK(!memcmp(&before,&p,sizeof p));
    changes[1]=(sj6_preset_field){116,.1f};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,changes,2)==SJ6_E_UNQUALIFIED_LINK);
    CHECK(!memcmp(&before,&p,sizeof p));
    changes[1]=(sj6_preset_field){36,1.32f};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,changes,2)==DNBX_E_RANGE);
    CHECK(!memcmp(&before,&p,sizeof p));
    changes[1]=(sj6_preset_field){185,NAN};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,changes,2)==DNBX_E_RANGE);
    CHECK(!memcmp(&before,&p,sizeof p));
    changes[1]=(sj6_preset_field){999,.1f};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,changes,2)==DNBX_E_RANGE);
    CHECK(!memcmp(&before,&p,sizeof p));
    CHECK(dnbx_store_raw(&p,115,1.f)==DNBX_RAW_STORED_ONLY); before=p;
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,&a,1)==SJ6_E_UNQUALIFIED_LINK);
    CHECK(!memcmp(&before,&p,sizeof p));
    changes[1]=(sj6_preset_field){115,0.f};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,changes,2)==DNBX_OK);
    const sj6_preset_field duplicate[]={{36,.25f},{36,.75f}};
    CHECK(sj6_apply_preset_fields(&p,SJ6_INDEPENDENT_ENVELOPES,duplicate,2)==DNBX_OK);
    float r,seconds;
    CHECK(dnbx_read_raw(&p,36,&r)==DNBX_OK && r==.75f);
    CHECK(dnbx_attack_from_unit(.75f,&seconds)==DNBX_OK);
    dnbx_layer_params layer;
    CHECK(dnbx_get_layer(&p,0,&layer)==DNBX_OK && layer.amp.attack_s==seconds);
    puts("PASS raw status, independent/unknown owner, later links, omitted retained link, invalid-batch atomicity, duplicate final field");
    return 0;
}
int main(void)
{
    if (absolute_attack_cases()) return 1;
    return admission_and_order();
}
