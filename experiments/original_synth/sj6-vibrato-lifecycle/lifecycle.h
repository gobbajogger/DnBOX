#ifndef SJ6_VIB_LIFECYCLE_H
#define SJ6_VIB_LIFECYCLE_H
/* Diagnostic/default-off source-matching kernel. Caller supplies the
 * authenticated elapsed transport delta, not a guessed sample-clock proxy.
 * Returned actions do not stand in for the source parameter selectors. */
enum { SJ6_VIB_ZERO_SENDS=1, SJ6_VIB_RESTART=2 };
typedef struct {
    double delay, elapsed;
    unsigned enabled, finished, new_note, lfo1_retrigger;
} sj6_vib_lifecycle;
static inline unsigned sj6_vib_key(sj6_vib_lifecycle *s,unsigned held_before){
    if (!s->enabled || held_before) return 0;
    s->elapsed=0.;
    s->finished=0;
    s->new_note=1;
    s->lfo1_retrigger=s->delay>=.01;
    return s->lfo1_retrigger ? SJ6_VIB_ZERO_SENDS : 0;
}
static inline unsigned sj6_vib_elapsed(sj6_vib_lifecycle *s,double tick) {
    if (!s->enabled) return 0;
    s->elapsed+=tick;
    if (!(s->elapsed>s->delay) || s->finished) return 0;
    s->finished=1;
    return s->delay>=.2 ? SJ6_VIB_RESTART : 0;
}
#endif
