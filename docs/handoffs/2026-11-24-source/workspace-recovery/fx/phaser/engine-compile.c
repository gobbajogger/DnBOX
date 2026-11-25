#include "x0x.h"
#include "../dsp/s1c/s1c_front.h"
#include "s1_native_phaser.h"
extern struct phaser_compile_receiver { unsigned kind; struct { struct { s1c e; } synthone; } dsp; } original_bass[2];
extern const sound_t *mot_base_snd;
extern volatile uint8_t note_lock_applied[2];
extern int live_s1_mod(const sound_t*,unsigned);
extern int s1_typed_tuple_editable(void);
#include "engine.inc"
