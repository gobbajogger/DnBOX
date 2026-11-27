#include "../dsp/fastmath.h"
enum { SJ6_NATIVE_DELAY_CAP=1024 };
#include "../dsp/juno6_delay_native.inc"
static int delay_owner(const original_runtime_part *p)
{
    return p && p->status==ORIGINAL_RUNTIME_OK && p->kind==ORIGINAL_SOUND_JUNO && p->dsp.juno.synth.typed;
}
int original_runtime_delay_bind(original_runtime_part *p,int16_t *arena,uint32_t epoch)
{
    if(!delay_owner(p)||!arena||!epoch||p->dsp.juno.lab_delay.arena) return 0;
    p->dsp.juno.lab_delay.epoch=epoch; p->dsp.juno.lab_delay.arena=arena; return 1;
}
int original_runtime_delay_publish(original_runtime_part *p,uint32_t epoch,unsigned version,const uint8_t *q,unsigned n)
{
    if(!delay_owner(p)||!p->dsp.juno.lab_delay.arena||epoch!=p->dsp.juno.lab_delay.epoch||
       p->dsp.juno.lab_delay.ready||version!=1||!q||n!=4||q[0]>3||((q[1]|q[2]|q[3])&128)) return 0;
    memcpy(p->dsp.juno.lab_delay.pending,q,4);
    p->dsp.juno.lab_delay.result=1; p->dsp.juno.lab_delay.ready=1;
    return 1;
}
int original_runtime_delay_result(const original_runtime_part *p)
{
    return delay_owner(p)?p->dsp.juno.lab_delay.result:0;
}
static void original_runtime_delay_drain(original_runtime_part *p,float bpm)
{
    if(!delay_owner(p)||!p->dsp.juno.lab_delay.arena) return;
    juno6_t *s=&p->dsp.juno.synth;
    if(s->delay24.tap && s->delay24.tap!=p->dsp.juno.lab_delay.arena) return;
    if(p->dsp.juno.lab_delay.ready) {
        const uint8_t *q=p->dsp.juno.lab_delay.pending;
        p->dsp.juno.lab_delay.ready=0;
        if(native_delay_frames(s,q[1],bpm,!!(q[0]&2u))) {
            memcpy(p->dsp.juno.lab_delay.applied,q,4);
            juno6_delay_native_arm(s,p->dsp.juno.lab_delay.arena);
            juno6_delay_native_config(s,q,bpm);
            p->dsp.juno.lab_delay.present=1; p->dsp.juno.lab_delay.last_bpm=bpm; p->dsp.juno.lab_delay.result=2;
        } else p->dsp.juno.lab_delay.result=3;
    }
    if(p->dsp.juno.lab_delay.present && bpm!=p->dsp.juno.lab_delay.last_bpm) {
        juno6_delay_native_config(s,p->dsp.juno.lab_delay.applied,bpm);
        p->dsp.juno.lab_delay.last_bpm=bpm;
    }
}
