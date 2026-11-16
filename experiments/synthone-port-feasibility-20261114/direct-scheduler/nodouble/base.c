/* Float overlay for AudioKit Soundpipe base.c. Upstream bytes are unchanged.
 * sp_midi2cps uses the 128-note table for integer notes and the firmware
 * exp2 polynomial for any other finite value. */
#include <string.h>
#include "float_math.h"
#include "midi_cps.h"
#include "slot.h"
#include "soundpipe.h"

static sp_data s1_sp_slot X0X_POOL;
static float s1_sp_out X0X_POOL;
static unsigned char s1_sp_used;

int sp_create(sp_data **spp)
{
    if (!spp || s1_sp_used) return SP_NOT_OK;
    s1_sp_used = 1;
    memset(&s1_sp_slot, 0, sizeof s1_sp_slot);
    s1_sp_out = 0.f;
    s1_sp_slot.out = &s1_sp_out;
    s1_sp_slot.nchan = 1;
    s1_sp_slot.sr = 44100;
    s1_sp_slot.len = 5ul * (unsigned long)s1_sp_slot.sr;
    memcpy(s1_sp_slot.filename, "test.wav", 9);
    *spp = &s1_sp_slot;
    return SP_OK;
}

int sp_createn(sp_data **spp, int nchan)
{
    if (nchan != 1) return SP_NOT_OK;
    return sp_create(spp);
}

int sp_destroy(sp_data **spp)
{
    if (spp && *spp == &s1_sp_slot) { s1_sp_used = 0; *spp = 0; }
    return SP_OK;
}

int sp_process(sp_data *sp, void *ud, void (*callback)(sp_data *, void *))
{
    (void)sp; (void)ud; (void)callback;
    return SP_NOT_OK;
}

int sp_process_raw(sp_data *sp, void *ud, void (*callback)(sp_data *, void *))
{
    (void)sp; (void)ud; (void)callback;
    return SP_NOT_OK;
}

int sp_process_plot(sp_data *sp, void *ud, void (*callback)(sp_data *, void *))
{
    (void)sp; (void)ud; (void)callback;
    return SP_NOT_OK;
}

int sp_auxdata_alloc(sp_auxdata *aux, size_t size)
{
    (void)size;
    if (aux) { aux->ptr = 0; aux->size = 0; }
    return SP_NOT_OK;
}

int sp_auxdata_free(sp_auxdata *aux)
{
    if (aux) { aux->ptr = 0; aux->size = 0; }
    return SP_OK;
}

SPFLOAT sp_midi2cps(SPFLOAT nn)
{
    if (nn >= 0.f && nn < 128.f) {
        int note = (int)nn;
        if ((float)note == nn) return s1_midi_cps_at(note);
    }
    return 440.f * s1_exp2f((nn - 69.f) / 12.f);
}

int sp_set(sp_param *p, SPFLOAT val)
{
    p->state = 1;
    p->val = val;
    return SP_OK;
}

int sp_out(sp_data *sp, uint32_t chan, SPFLOAT val)
{
    if (!sp || !sp->out || chan > (uint32_t)sp->nchan - 1) return SP_NOT_OK;
    sp->out[chan] = val;
    return SP_OK;
}

uint32_t sp_rand(sp_data *sp)
{
    uint32_t val = (1103515245 * sp->rand + 12345) % SP_RANDMAX;
    sp->rand = val;
    return val;
}

void sp_srand(sp_data *sp, uint32_t val)
{
    sp->rand = val;
}
