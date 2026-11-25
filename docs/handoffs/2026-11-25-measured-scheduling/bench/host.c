#define AUDIO_PATH_IDENTITY_LIBRARY
#include "audio_path_identity.c"
#if DNBX_HOST_V808_BENCH
extern volatile uint32_t eng_bench_virtual808_spans;
#define BENCH_COUNT eng_bench_virtual808_spans
#else
#define BENCH_COUNT 0u
#endif
int main(void)
{
    boot();
    engine_guard_enable(0);
    assert(!plat_cpu2_set(1));
    assert(!engine_bass_select_preset(0,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
    int32_t lr[512];
    for(unsigned i=0;i<16;i++) engine_render(lr,256);
    assert(engine_bass_bank(0)==ORIGINAL_SOUND_SYNTHONE);
    assert(engine_bass_bank(1)==ORIGINAL_SOUND_CHICAGO);
    engine_arp_settings set;
    assert(engine_arp_settings_read(0,&set));
    assert(engine_arp_set(0,set.revision,ARP_F_ON,1)==S1_EDIT_QUEUED);
    engine_render(lr,256);
    assert(engine_arp_settings_read(0,&set) && set.on);
    for(unsigned i=0;i<180;i++) {
        now_ms=i*256000u/44100u;
        if(i==52) assert(!plat_cpu2_set(0)); /* synchronous no dispatch fallback */
        if(i==64) assert(!plat_cpu2_set(1));
        if(i%6==0) engine_drum(0,7,1.f);
        if(i%6==2) engine_drum(0,8,.9f);
        if(i%4==0) {
            engine_bass_on_velocity(0,48,0,0,100);
            engine_bass_on_velocity(1,36,0,0,110);
        }
        if(i%4==3) {
            engine_bass_off_note(0,48);
            engine_bass_off_note(1,36);
        }
        if(i>=80 && i<130 && i%4==0) engine_drum(1,0,.85f);
        uint32_t before=BENCH_COUNT;
        engine_render(lr,256);
        uint32_t after=BENCH_COUNT;
        if(DNBX_HOST_V808_BENCH && i>=85 && i<125)
            assert(after==before); /* active 808 observed; no virtual stage */
        put(stdout,lr,sizeof lr);
        put(stdout,eng_peak,sizeof eng_peak);
        put(stdout,eng_step,sizeof eng_step);
    }
    if(DNBX_HOST_V808_BENCH) {
        assert(BENCH_COUNT>10);
        fprintf(stderr,"virtual inactive 808 spans=%u\n",BENCH_COUNT);
    }
}
