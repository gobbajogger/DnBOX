/* Independent host probe. Only public engine inputs mutate the test state. */
#define SMC_UI_MAIN retained_ui_tests
#include "smc_ui_test.c"
#include "app/original_runtime.h"

static juno6_t *parts[2];
static int initialized;
void __real_juno6_init(juno6_t *,float);
void __wrap_juno6_init(juno6_t *s, float rate)
{
    __real_juno6_init(s,rate);
    if (initialized < 2) parts[initialized++] = s;
}

static void block(unsigned frames)
{
    int32_t stereo[512];
    engine_render(stereo, frames);
}
static void select_preset(int owner,int preset)
{
    CHECK(engine_bass_select_preset(owner,ORIGINAL_SOUND_JUNO,preset,&proj.sound)==0);
    for(unsigned i=0;i<1024 && engine_bass_preset_pending(owner);++i) block(256);
    CHECK(engine_bass_bank(owner)==ORIGINAL_SOUND_JUNO && !engine_bass_preset_pending(owner));
}
static unsigned mode(unsigned o,unsigned l)
{
    dnbx_layer_params p;
    CHECK(dnbx_get_layer(&parts[o]->core,l,&p)==DNBX_OK);
    return p.oscillator_mode;
}
static void raw(unsigned o,unsigned id,float expected)
{
    float value;
    CHECK(dnbx_read_raw(&parts[o]->core,id,&value)==DNBX_OK && !memcmp(&value,&expected,4));
}
int main(void)
{
    setup(V_PART,PART_303A);
    CHECK(original_sound_factories(ORIGINAL_SOUND_JUNO)==4);
    for(int o=0;o<2;o++)select_preset(o,0);
    CHECK(initialized==2 && parts[0]!=parts[1]);
    const engine_superj6_pwm narrow={.75f,1.f},normal={0.f,0.f};
    for(unsigned o=0;o<2;++o) {
        unsigned other=o^1;
        for(unsigned l=0;l<2;l++)CHECK(mode(o,l)==DNBX_OSC_STOCK_BLEP);
        sound_t before=proj.sound;
        CHECK(engine_superj6_pre_note_wave(o,0,1,&narrow));
        CHECK(engine_superj6_pre_note_wave(o,0,1,&normal));
        CHECK(engine_superj6_edit(o,ENGINE_SJ6_AMP2_ATTACK,.25f));
        block(0);
        CHECK(parts[o]->pending_wave_mask==1 && mode(o,0)==DNBX_OSC_STOCK_BLEP);
        engine_project_hold(1);
        CHECK(!engine_superj6_pre_note_wave(o,0,1,&narrow));
        block(32);
        CHECK(parts[o]->pending_wave_mask==1 && mode(o,0)==DNBX_OSC_STOCK_BLEP);
        engine_project_release();
        engine_bass_on_velocity(o,60,0,0,100);
        block(32);
        CHECK(!parts[o]->pending_wave_mask && mode(o,0)==DNBX_OSC_SUPERJ6);
        CHECK(mode(o,1)==DNBX_OSC_STOCK_BLEP && juno6_active(parts[o]));
        raw(o,78,0); raw(o,210,0); raw(o,119,.25f);
        CHECK(!engine_superj6_pre_note_wave(o,0,0,&normal));
        engine_bass_off(o);block(8);
        CHECK(juno6_active(parts[o]));
        CHECK(!engine_superj6_pre_note_wave(o,0,0,&normal));
        engine_set(T_303,o,0,64); block(1);
        CHECK(mode(o,0)==DNBX_OSC_STOCK_BLEP && mode(o,1)==DNBX_OSC_STOCK_BLEP);
        CHECK(!memcmp(&proj.sound,&before,sizeof before));
        CHECK(mode(other,0)==DNBX_OSC_STOCK_BLEP && mode(other,1)==DNBX_OSC_STOCK_BLEP);
        for(unsigned b=0;b<4096 && juno6_active(parts[o]);++b)block(256);
        CHECK(!juno6_active(parts[o]));
        CHECK(engine_superj6_pre_note_wave(o,0,1,&narrow));
        block(1);raw(o,78,.75f);CHECK(mode(o,0)==DNBX_OSC_SUPERJ6);
        select_preset(o,1);
        float ignored;
        CHECK(mode(o,0)==DNBX_OSC_STOCK_BLEP && dnbx_read_raw(&parts[o]->core,78,&ignored)==DNBX_E_ABSENT);
    }
    printf("two-owner compact lifecycle: %u assertions PASS\n",checks);
}
