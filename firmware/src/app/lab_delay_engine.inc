/* SPDX-License-Identifier: GPL-3.0-only */
/* No extra queue: publish the requested record into the runtime mailbox
 * under the existing audio-source mask, only after audio bound this owner. */
_Static_assert(sizeof original_fx[0].arena.history == 4096, "exact owner Delay arena");
static uint32_t lab_delay_epoch[NBASS];
static int lab_delay_engine_ready(unsigned v)
{
    const original_runtime_part *p=&original_bass[v];
    return !project_hold && original_fx_ready && lab_delay_epoch[v] &&
           lab_delay_epoch[v]!=UINT32_MAX && p->status==ORIGINAL_RUNTIME_OK &&
           p->kind==ORIGINAL_SOUND_JUNO && p->dsp.juno.synth.typed;
}
uint32_t engine_lab_delay_epoch(unsigned v)
{
    if(v>=NBASS)return 0;
    uint32_t irq=plat_cq_lock();
    uint32_t epoch=0;
    if(lab_delay_engine_ready(v) && original_bass[v].dsp.juno.lab_delay.arena)
        epoch=lab_delay_epoch[v];
    plat_cq_unlock(irq); return epoch;
}
int engine_lab_delay_publish(unsigned v,uint32_t epoch,unsigned version,const uint8_t *q,unsigned n)
{
    if(v>=NBASS)return 0;
    uint32_t irq=plat_cq_lock();
    int ok=lab_delay_engine_ready(v) &&
           original_runtime_delay_publish(&original_bass[v],epoch,version,q,n);
    plat_cq_unlock(irq); return ok;
}
/* Existing cold adoption sites only; retire invalidates both pending intent
 * and ring ownership before the compact history can be reused. */
static int lab_delay_owner_retire(unsigned v)
{
    original_runtime_part *p=&original_bass[v];
    int used=p->kind==ORIGINAL_SOUND_JUNO && p->dsp.juno.synth.delay24.tap!=NULL;
    int rc=original_runtime_retire(p);
    if(rc==ORIGINAL_RUNTIME_OK){
        if(lab_delay_epoch[v]!=UINT32_MAX) ++lab_delay_epoch[v];
        if(used) (void)original_chicago_fx2_compact_init(&original_fx[v]);
    }
    return rc;
}
static void lab_delay_bind_current(void)
{
    for(unsigned v=0;v<NBASS;++v) if(lab_delay_engine_ready(v)) {
        original_runtime_part *p=&original_bass[v];
        if(!p->dsp.juno.lab_delay.arena)
            (void)original_runtime_delay_bind(p,original_fx[v].arena.history,lab_delay_epoch[v]);
    }
}
