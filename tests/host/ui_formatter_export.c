/* SPDX-License-Identifier: GPL-3.0-only */
/* Exact production formatter readout at the boot factory state, not runtime admission. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void json_string(const char *s)
{
    putchar('"');for(;*s;s++){unsigned char c=(unsigned char)*s;if(c=='"'||c=='\\'){putchar('\\');putchar(c);}else if(c<32)printf("\\u%04x",c);else putchar(c);}putchar('"');
}
int main(void)
{
    setup(V_PART,PART_303A);
    for(int voice=0;voice<2;voice++) {
        int bank=engine_bass_bank(voice);
        CHECK(bank==ORIGINAL_SOUND_CHICAGO || bank==ORIGINAL_SOUND_MIAMI);
        printf("FORMATTER:{\"schema\":\"dnbox-production-formatter/1\",\"bank\":%d,\"engine\":\"%s\",\"voice\":%d,\"target\":%d,\"preset\":%d,\"runtimeAdmission\":\"NOT_ADMITTED\",\"state\":\"boot factory preset\",\"controls\":[",bank,bank==ORIGINAL_SOUND_CHICAGO?"Chicago":"Miami",voice,T_303,engine_bass_preset_index(voice));
        int first=1;
        for(int parameter=0;parameter<128;parameter++) {
            pref_t r=PR(R_ENG,T_303,voice,parameter); const x0x_param_t *d=pref_desc(r);
            if(!d)continue;
            if(!first)putchar(',');first=0;
            printf("{\"slot\":%d,\"name\":",parameter);json_string(d->name);
            printf(",\"maximumRaw\":%d,\"values\":[",d->max);
            for(int raw=0;raw<=d->max;raw++) {
                char number[64],unit[64];pref_value_of(r,raw,number,unit);
                if(raw)putchar(',');printf("{\"raw\":%d,\"number\":",raw);json_string(number);printf(",\"unit\":");json_string(unit);putchar('}');
            }
            printf("]}");
        }
        printf("]}\n");
    }
    return 0;
}
