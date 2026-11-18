/* SPDX-License-Identifier: GPL-3.0-only */
/* Exact production formatter readout at the boot factory state, not runtime admission. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void json_string(const char *s)
{
    putchar('"');for(;*s;s++){unsigned char c=(unsigned char)*s;if(c=='"'||c=='\\'){putchar('\\');putchar(c);}else if(c<32)printf("\\u%04x",c);else putchar(c);}putchar('"');
}
static void export_drum_voice(int target,int voice)
{
    int part=target==T_909?PART_909:PART_808;
    setup(V_PART,part);if(voice<NDRUM)ui.sel[part]=(uint8_t)voice;build_pages();
    int count=engine_nparams(target,voice),state[128];CHECK(count>0 && count<=128);
    for(int slot=0;slot<count;slot++)state[slot]=pref_get(PR(R_ENG,target,voice,slot));
    printf("DRUM_FORMATTER:{\"schema\":\"dnbox-production-formatter/1\",\"engine\":\"%s\",\"target\":%d,\"voice\":%d,\"voiceName\":",target==T_909?"909":"808",target,voice);
    json_string(voice<NDRUM?engine_voice_name(target,voice):"KIT");
    printf(",\"qualification\":\"FORMATTER_ONLY_NOT_DEVICE_OR_FULL_LANE\",\"stateScope\":\"boot factory state; other controls fixed while formatting each raw domain\",\"stateVector\":[");
    for(int slot=0;slot<count;slot++){if(slot)putchar(',');printf("{\"slot\":%d,\"raw\":%d}",slot,state[slot]);}
    printf("],\"controls\":[");
    for(int slot=0;slot<count;slot++) {
        pref_t r=PR(R_ENG,target,voice,slot);const x0x_param_t *d=pref_desc(r);CHECK(d!=0);
        if(slot)putchar(',');printf("{\"kind\":%d,\"target\":%d,\"voice\":%d,\"slot\":%d,\"nativeName\":",r.kind,target,voice,slot);json_string(d->name);
        printf(",\"displayName\":");json_string(pref_name(r));printf(",\"minimumRaw\":0,\"maximumRaw\":%d,\"values\":[",d->max);
        for(int raw=0;raw<=d->max;raw++) {
            char number[64],unit[64];pref_value_of(r,raw,number,unit);
            if(raw)putchar(',');printf("{\"raw\":%d,\"number\":",raw);json_string(number);printf(",\"unit\":");json_string(unit);putchar('}');
        }
        printf("]}");
    }
    printf("]}\n");
    for(int slot=0;slot<count;slot++)CHECK(pref_get(PR(R_ENG,target,voice,slot))==state[slot]);
}
int main(void)
{
    setup(V_PART,PART_303A);
    for(int n=0;n<3;n++) {
        int mix=n==2,voice=mix?0:n,target=mix?T_MIX:T_303;
        int bank=mix?-1:engine_bass_bank(voice);
        CHECK(mix || bank==ORIGINAL_SOUND_CHICAGO || bank==ORIGINAL_SOUND_MIAMI);
        printf("FORMATTER:{\"schema\":\"dnbox-production-formatter/1\",\"bank\":%d,\"engine\":\"%s\",\"voice\":%d,\"target\":%d,\"preset\":%d,\"runtimeAdmission\":\"NOT_ADMITTED\",\"state\":\"boot factory preset\",\"controls\":[",bank,mix?"Mixer":bank==ORIGINAL_SOUND_CHICAGO?"Chicago":"Miami",voice,target,mix?-1:engine_bass_preset_index(voice));
        int first=1;
        for(int parameter=0;parameter<128;parameter++) {
            pref_t r=PR(R_ENG,target,voice,parameter); const x0x_param_t *d=pref_desc(r);
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
    for(int target=T_909;target<=T_808;target++)for(int voice=0;voice<=NDRUM;voice++)export_drum_voice(target,voice);
    printf("PASS formatter export without control mutation: %u checks\n",checks);
    return 0;
}
