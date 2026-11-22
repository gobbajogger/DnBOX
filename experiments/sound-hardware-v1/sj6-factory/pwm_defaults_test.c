/* Synthetic transitions for the compact application fixture. pwm_expected.inc
 * holds the main-pass words from the pinned, source-derived Conductor model. */
#define SJ6_APPLICATION_MAIN check_real_factory
#include "compact_application.c"
#include <math.h>
#include "pwm_expected.inc"

int main(void)
{
    unsigned authored = 0, defaulted = 0;
    open_parts();
    for (unsigned v = 0; v < 2; ++v) {
        for (unsigned p = 0; p < sizeof expected / sizeof expected[0]; ++p) {
            /* Dirty both layers on this owner before every selection. */
            app_fields fields;
            CHECK(select_fields(0, 111, &fields));  /* square, all four nonzero */
            apply_fields(v, &fields, &authored, &defaulted);
            CHECK(select_fields(0, p, &fields));
            CHECK(((fields.present >> 6) & 15u) == (p & 15u));
            apply_fields(v, &fields, &authored, &defaulted);
            for (unsigned l = 0; l < 2; ++l) {
                float amount, enable;
                memcpy(&amount, &expected[p][l], sizeof amount);
                memcpy(&enable, &expected[p][2+l], sizeof enable);
                check_raw(v, 78 + l, amount);
                check_raw(v, 210 + l, enable);
                dnbx_layer_params actual;
                CHECK(dnbx_get_layer(&owner[v]->core, l, &actual) == DNBX_OK);
                double duty = fields.wave[l] == 1.f && enable > .5f ? .5 - .47 * amount : .5;
                CHECK(fabs((double)actual.pulse_width - duty) < 6e-8);
            }
        }
    }
    CHECK(authored == 5120 && defaulted == 512);
    printf("synthetic_records=128 owners=2 applications=512 authored_words=%u defaulted_pwm_words=%u\n",
           authored, defaulted);
    return 0;
}
