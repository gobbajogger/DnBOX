/* SPDX-License-Identifier: GPL-3.0-only */
/* Bounded actual-app transaction diagnostic, not an admitted device catalog. */
#include "dnr2_reference/remote_core.h"
#include <ctype.h>
static dnr2_core protocol;
static int protocol_ready;
void dnbox_source_changed(void);
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#include "dnr2_reference/remote_core.c"
#include "dnr2_reference/remote_wire.c"
void dnbox_source_changed(void)
{
    if(protocol_ready) {
        uint32_t flags=plat_cq_lock();dnr2_changed(&protocol);plat_cq_unlock(flags);
    }
}
/* Host qualification table only; never linked into a firmware endpoint. */
typedef struct {uint16_t id;uint8_t target,voice,parameter;int8_t bank;uint16_t maximum;char label[64];} actual_control;
static actual_control controls[512];static unsigned control_count;static uint32_t catalog_crc;
static unsigned makeup_id;
static void collect_actual(unsigned target,unsigned voice,int bank)
{
    for(unsigned parameter=0;parameter<(unsigned)engine_nparams(target,voice);parameter++) {
        if(target==T_303 && original_sound_is_performance((unsigned)bank,parameter))continue;
        pref_t ref=PR(R_ENG,target,voice,parameter);const x0x_param_t *d=pref_desc(ref);
        CHECK(d && !pref_read_only(ref));CHECK(control_count<512);
        actual_control *c=&controls[control_count++];c->id=(uint16_t)control_count;
        c->target=target;c->voice=voice;c->parameter=parameter;c->bank=(int8_t)bank;c->maximum=d->max;
        const char *label=pref_name(ref);CHECK(label && strlen(label)<sizeof c->label);strcpy(c->label,label);
        CHECK(!strchr(label,'\n') && !strchr(label,'|'));
        if(target==T_MST && voice==0 && parameter==MST_MAKEUP)makeup_id=c->id;
    }
}
static uint32_t crc_byte(uint32_t c,unsigned b)
{c^=b;for(unsigned i=0;i<8;i++)c=(c>>1)^((c&1)?0xedb88320u:0);return c;}
static void make_actual_catalog(void)
{
    for(unsigned t=0;t<NTARGETS;t++)if(t!=T_303)
        for(unsigned v=0;v<(unsigned)engine_nvoices(t);v++)collect_actual(t,v,-1);
    for(unsigned bank=ORIGINAL_SOUND_CHICAGO;bank<=ORIGINAL_SOUND_MIAMI;bank++)
        for(unsigned v=0;v<NBASS;v++) {
            CHECK(engine_bass_select_preset(v,bank,0,&proj.sound)==0);run_ms(100);
            CHECK(!engine_bass_preset_pending(v) && engine_bass_bank(v)==(int)bank);
            collect_actual(T_303,v,(int)bank);
        }
    for(unsigned v=0;v<NBASS;v++) {CHECK(engine_bass_select_preset(v,ORIGINAL_SOUND_CHICAGO,0,&proj.sound)==0);run_ms(100);}
    CHECK(control_count==326 && makeup_id);
    uint32_t crc=0xffffffffu;const char *schema="dnbox-actual-scalar-host-v1";
    do {crc=crc_byte(crc,(unsigned char)*schema);}while(*schema++);
    for(unsigned i=0;i<control_count;i++) {
        const actual_control *c=&controls[i];uint32_t fields[]={c->id,DNR2_U32,c->target,c->voice,c->parameter,(uint32_t)(c->bank+1),0,c->maximum};
        for(unsigned k=0;k<sizeof fields/sizeof *fields;k++)for(unsigned byte=0;byte<4;byte++)crc=crc_byte(crc,(fields[k]>>(byte*8))&255);
        const char *label=c->label;do {crc=crc_byte(crc,(unsigned char)*label);}while(*label++);
    }
    catalog_crc=~crc;CHECK(catalog_crc);
}
static const actual_control *lookup_actual(uint16_t id)
{return id && id<=control_count?&controls[id-1]:0;}
static int read_actual(void *ctx,uint16_t id,uint8_t *kind,uint32_t *value)
{
    (void)ctx;const actual_control *c=lookup_actual(id);
    if(!c || ui_project_busy())return 0;
    if(c->target==T_303 && (c->bank!=engine_bass_bank(c->voice) || c->bank!=engine_bass_sound_bank(&proj.sound,c->voice) ||
        engine_bass_preset_pending(c->voice) || engine_bass_status(c->voice)!=ORIGINAL_RUNTIME_OK || engine_bass_sound_validate(&proj.sound)))return 0;
    pref_t ref=PR(R_ENG,c->target,c->voice,c->parameter);const x0x_param_t *d=pref_desc(ref);
    if(!d || d->max!=c->maximum || pref_read_only(ref))return 0;
    int current=pref_get(ref);if(current<0 || current>c->maximum)return 0;
    *kind=DNR2_U32;*value=(uint32_t)current;return 1;
}
static int set_actual(void *ctx,uint16_t id,uint8_t kind,uint32_t value)
{
    (void)ctx;const actual_control *c=lookup_actual(id);
    if(!c || kind!=DNR2_U32 || value>c->maximum)return 0;
    return dnbox_remote_scalar_set(c->target,c->voice,c->parameter,c->bank,value);
}
static int no_down(void *ctx,uint16_t id,uint32_t value,uint32_t *cookie)
{(void)ctx;(void)id;(void)value;(void)cookie;return 0;}
static void no_up(void *ctx,uint32_t cookie){(void)ctx;(void)cookie;}
static dnr2_reply transact(dnr2_command *c)
{
    uint32_t flags=plat_cq_lock();
    dnr2_reply r=dnr2_handle(&protocol,c,plat_ms());
    /* Cached command results never substitute for the current source revision. */
    r.revision=protocol.revision;
    plat_cq_unlock(flags);return r;
}
static void json_text(const char *s)
{
    putchar('"');for(;*s;s++) {
        unsigned char c=(unsigned char)*s;
        if(c=='"'||c=='\\') {putchar('\\');putchar(c);}
        else if(c<32)printf("\\u%04x",c);else putchar(c);
    }putchar('"');
}
static int dump_actual_catalog(void)
{
    printf("{\"schema\":\"dnbox-actual-scalar-host-v1\",\"admitted\":false,\"nativeAdmitted\":false,\"testOnly\":true,\"crc\":%u,\"count\":%u,\"diagnosticMasterMakeup\":%u,\"controls\":[",catalog_crc,control_count,makeup_id);
    for(unsigned i=0;i<control_count;i++) {
        const actual_control *c=&controls[i];
        printf("%s{\"id\":%u,\"kind\":1,\"min\":0,\"max\":%u,\"target\":%u,\"voice\":%u,\"parameter\":%u,\"bank\":%d,\"label\":",i?",":"",c->id,c->maximum,c->target,c->voice,c->parameter,c->bank);
        json_text(c->label);putchar('}');
    }
    puts("]}");return 0;
}
static void actual_state(void)
{
    uint32_t flags=plat_cq_lock();
    int value=pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP));uint32_t revision=protocol.revision;unsigned available=engine_queue_available_locked();
    plat_cq_unlock(flags);
    printf("{\"value\":%d,\"revision\":%u,\"queueFree\":%u,\"admitted\":false}\n",
        value,revision,available);
}
static int serve_actual(void)
{
    char line[1024];
    while(fgets(line,sizeof line,stdin)) {
        unsigned value;char extra;
        if(sscanf(line,"external %u %c",&value,&extra)==1) {
            const x0x_param_t *d=pref_desc(PR(R_ENG,T_MST,0,MST_MAKEUP));
            if(value>d->max)puts("{\"invalid\":true}");
            else { pref_set(PR(R_ENG,T_MST,0,MST_MAKEUP),(int)value);actual_state(); }
        } else if(sscanf(line,"advance %u %c",&value,&extra)==1 && value<=5000) {
            run_ms((int)value);uint32_t flags=plat_cq_lock();dnr2_tick(&protocol,plat_ms());plat_cq_unlock(flags);actual_state();
        } else {
            size_t n=strcspn(line,"\r\n");uint8_t in[DNR2_FRAME_BYTES],out[DNR2_FRAME_BYTES];int valid=n==102;
            for(unsigned i=0;i<DNR2_FRAME_BYTES && valid;i++) {
                char pair[3]={line[2*i],line[2*i+1],0};
                if(!isxdigit((unsigned char)pair[0])||!isxdigit((unsigned char)pair[1]))valid=0;
                else in[i]=(uint8_t)strtoul(pair,0,16);
            }
            dnr2_command c;
            if(!valid||!dnr2_decode(&c,in,sizeof in))puts("{\"invalid\":true}");
            else {
                dnr2_reply r=transact(&c);
                if(!dnr2_encode_reply(out,c.op,&r))puts("{\"invalid\":true}");
                else { printf("{\"hex\":\"");for(unsigned i=0;i<sizeof out;i++)printf("%02x",out[i]);puts("\",\"admitted\":false}"); }
            }
        }
        fflush(stdout);
    }
    return ferror(stdin)?1:0;
}
int main(int argc,char **argv)
{
    setup(V_HOME,PART_303A);run_ms(100);
    make_actual_catalog();
    dnr2_host host={0,read_actual,set_actual,no_down,no_up,catalog_crc,(uint16_t)control_count};
    uint32_t high[3]={2,3,4};CHECK(dnr2_init(&protocol,&host,1,high));protocol_ready=1;
    if(argc==2 && !strcmp(argv[1],"--serve"))return serve_actual();
    if(argc==2 && !strcmp(argv[1],"--catalog"))return dump_actual_catalog();
    CHECK(argc==1);
    dnr2_command c={0};memcpy(c.unit_high,high,sizeof high);c.unit=1;c.client=7;c.request=1;c.version=2;c.op=DNR2_OPEN;
    dnr2_reply r=transact(&c);CHECK(r.status==DNR2_OK);c.epoch=r.epoch;
    c.request++;c.op=DNR2_SET;c.revision=r.revision;c.control=(uint16_t)makeup_id;c.value=23;
    uint32_t previous=protocol.revision;r=transact(&c);
    CHECK(r.status==DNR2_QUEUED);CHECK(r.value==23);CHECK(r.revision==previous+1);
    CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==23);
    /* A repeated exact request must not reapply, but cannot hide an external edit. */
    pref_set(PR(R_ENG,T_MST,0,MST_MAKEUP),24);
    uint32_t external=protocol.revision;CHECK(external>r.revision);
    r=transact(&c);CHECK(r.revision==external);
    c.request++;c.value=25; /* stale caller cannot overwrite external mutation */
    r=transact(&c);CHECK(r.status==DNR2_CONFLICT);CHECK(protocol.revision==external);
    CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==24);
    c.request++;c.revision=external;c.value=0xffffffffu;
    r=transact(&c);CHECK(r.status==DNR2_BAD);CHECK(protocol.revision==external);
    CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==24);
    run_ms(100);
    for(unsigned i=0;i<1024;i++)engine_set_from_remote(T_MST,0,MST_MAKEUP,24);
    CHECK(engine_queue_available_locked()==0);
    c.request++;c.value=25;r=transact(&c);CHECK(r.status==DNR2_BAD);
    CHECK(protocol.revision==external);CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==24);
    run_ms(500);CHECK(engine_queue_available_locked()==1024);
    c.request++;r=transact(&c);CHECK(r.status==DNR2_QUEUED);
    CHECK(protocol.revision==external+1);CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==25);
    unsigned actual_edits=0, inactive_reads=0;
    for(unsigned bank=ORIGINAL_SOUND_CHICAGO;bank<=ORIGINAL_SOUND_MIAMI;bank++) {
        for(unsigned v=0;v<NBASS;v++) {CHECK(engine_bass_select_preset(v,bank,0,&proj.sound)==0);run_ms(100);CHECK(!engine_bass_preset_pending(v));}
        for(unsigned i=0;i<control_count;i++) {
            const actual_control *d=&controls[i];uint8_t kind;uint32_t value;
            int available=read_actual(0,d->id,&kind,&value);
            if(d->bank>=0 && d->bank!=(int)bank) {CHECK(!available);inactive_reads++;continue;}
            CHECK(available && kind==DNR2_U32 && value<=d->maximum);
            if(bank!=ORIGINAL_SOUND_CHICAGO && d->bank<0)continue;
            unsigned values[]={0,d->maximum/2,d->maximum};
            for(unsigned k=0;k<3;k++) {
                c.control=d->id;c.value=values[k];c.revision=protocol.revision;c.request++;
                uint32_t before=protocol.revision;r=transact(&c);
                CHECK(r.status==DNR2_QUEUED);CHECK(r.revision==before+1);CHECK(r.value==values[k]);
                CHECK(read_actual(0,d->id,&kind,&value) && value==values[k]);
                run_ms(20);actual_edits++;
            }
        }
    }
    CHECK(actual_edits==978 && inactive_reads==72);
    printf("PASS actual scalar catalog: %u controls, %u protocol edits, %u inactive bank reads rejected\n",control_count,actual_edits,inactive_reads);
    printf("PASS actual app/core transaction: %u checks; actual generated scalar table; no endpoint admission\n",checks);
    return 0;
}
