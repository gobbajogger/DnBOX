#include "x0x.h"
#include "s1_native_phaser.h"
unsigned phaser_position(const sound_t *s,unsigned v,unsigned k) {
    return k<S1_PHASE_COUNT?s1_phase_position(k,s1_phase_read(s,v)[k]):0;
}
float phaser_value(const sound_t *s,unsigned v,unsigned k) {
    return k<S1_PHASE_COUNT?s1_phase_value(k,s1_phase_read(s,v)[k]):0.f;
}
void phaser_set(sound_t *s,unsigned v,unsigned k,unsigned pos) {
    if(k<S1_PHASE_COUNT && pos<=S1_PHASE_MAX)s1_phase_write(s,v)[k]=(uint8_t)s1_phase_encode(k,pos);
}
