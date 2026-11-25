#ifndef DNBOX_S1_FX_STIMULUS_H
#define DNBOX_S1_FX_STIMULUS_H
#define S1AB_RATE 44100
#define S1AB_CHUNK 128u
#define S1AB_LEAD 128u
#define S1AB_NOTE_LEN 33075u
#define S1AB_NOTES 6u
#define S1AB_TAIL (2u*S1AB_NOTE_LEN)
#define S1AB_FRAMES (S1AB_NOTES*S1AB_NOTE_LEN+S1AB_TAIL)
static const unsigned s1ab_note[S1AB_NOTES]={48,55,60,64,67,72};
static inline unsigned s1ab_vel(unsigned i) {return 64u+11u*i;}
enum { MODE_WET,MODE_DRY,MODE_DELAY,MODE_REVERB };
#endif
