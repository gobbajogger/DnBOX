/* Addresses only. The firmware keep calls this so --gc-sections retains every
 * .pool slot and the link map shows their bytes. It does not render audio. */
void *s1_pool_slot_adsr(void);
void *s1_pool_slot_butbp(void);
void *s1_pool_slot_buthp(void);
void *s1_pool_slot_crossfade(void);
void *s1_pool_slot_fosc(void);
void *s1_pool_slot_moog(void);
void *s1_pool_slot_noise(void);
void *s1_pool_slot_osc(void);
void *s1_pool_slot_morph(void);
void *s1_pool_slot_phasor(void);
void *s1_pool_slot_port(void);
void *s1_pool_slot_sp(void);
void *s1_pool_slot_spout(void);

void s1_pool_keep(void)
{
    void *volatile held = 0;
    held = s1_pool_slot_adsr();
    held = s1_pool_slot_butbp();
    held = s1_pool_slot_buthp();
    held = s1_pool_slot_crossfade();
    held = s1_pool_slot_fosc();
    held = s1_pool_slot_moog();
    held = s1_pool_slot_noise();
    held = s1_pool_slot_osc();
    held = s1_pool_slot_morph();
    held = s1_pool_slot_phasor();
    held = s1_pool_slot_port();
    held = s1_pool_slot_sp();
    held = s1_pool_slot_spout();
    (void)held;
}
