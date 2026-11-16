/* SPDX-License-Identifier: GPL-3.0-only */
#include "remote_scalar.h"
#include "original_runtime.h"
X0X_COLD int dnbox_remote_scalar_set(unsigned target, unsigned voice,
                                    unsigned parameter, int bank, unsigned value)
{
    uint32_t flags=plat_cq_lock();
    int accepted=0;
    if(project_busy || target>=NTARGETS || voice>=(unsigned)engine_nvoices((int)target) ||
       parameter>=NPARAMS_MAX || parameter>=(unsigned)engine_nparams((int)target,(int)voice))goto done;
    pref_t ref=PR(R_ENG,target,voice,parameter);
    const x0x_param_t *d=pref_desc(ref);
    if(!d || value>d->max || pref_read_only(ref))goto done;
    if(target==T_303) {
        /* Legacy Drone/Juno metadata must not be relabelled as the requested
         * replacement engines. Those engines need their own admitted adapter. */
        if((bank!=ORIGINAL_SOUND_CHICAGO && bank!=ORIGINAL_SOUND_MIAMI) ||
           bank!=engine_bass_bank((int)voice) ||
           bank!=engine_bass_sound_bank(&proj.sound,(int)voice) ||
           engine_bass_preset_pending((int)voice) ||
           engine_bass_status((int)voice)!=ORIGINAL_RUNTIME_OK ||
           engine_bass_sound_validate(&proj.sound))goto done;
        uint8_t cell[ORIGINAL_SOUND_BYTES];
        memcpy(cell,proj.sound.v[T_303][voice],sizeof cell);
        if(!original_sound_set(cell,parameter,value) || cell[parameter]!=value)goto done;
    } else if(bank!=-1)goto done;
    /* R_ENG emits exactly one queue command. Keep the existing source lock
     * from capacity check through mirror update and enqueue, so rejection
     * cannot leave a changed mirror or claim acceptance of a dropped command. */
    if(!engine_queue_available_locked())goto done;
    pref_set_impl(ref,(int)value,0);
    accepted=2;
done:
    plat_cq_unlock(flags);
    return accepted;
}
