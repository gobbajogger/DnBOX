/* Protocol-free lab receiver. Call only on the audio owner at a quiesced boundary. */
#if X0X_SJ6_NATIVE_DELAY && X0X_SUPERJ6_FACTORY
enum { SJ6_NATIVE_DELAY_CAP=1024 };
#include "juno6_delay_native.inc"

enum sj6d_status { SJ6D_BAD, SJ6D_UNAVAILABLE, SJ6D_STALE, SJ6D_RANGE, SJ6D_APPLIED };
typedef struct {
    uint32_t epoch;
    uint8_t present, q[4];
    float applied_bpm;
} sj6d_lane;

/* Arena and epoch are trusted host binding, never supplied by control packet.
 * Applies full transaction; no edits to project, ARP, preset, or Room. */
static enum sj6d_status sj6d_accept(sj6d_lane *lane, juno6_t *s, int16_t *arena,
    uint32_t epoch, unsigned version, const uint8_t *q, unsigned len, float bpm)
{
    if(!s || !s->typed || !arena || (s->delay24.tap && s->delay24.tap!=arena)) return SJ6D_UNAVAILABLE;
    if(epoch!=lane->epoch) return SJ6D_STALE;
    if(version!=1 || len!=4 || !q || q[0]>3u || ((q[1]|q[2]|q[3])&128u)) return SJ6D_BAD;
    if(!native_delay_frames(s,q[1],bpm,!!(q[0]&2u))) return SJ6D_RANGE;
    for(unsigned i=0;i<4;i++) lane->q[i]=q[i];
    juno6_delay_native_arm(s,arena);
    juno6_delay_native_config(s,lane->q,bpm);
    lane->applied_bpm=bpm;
    lane->present=1;
    return SJ6D_APPLIED;
}
/* Commit current timeline tempo immediately before render, not guessed wall time.
 * Out-of-range clock remains dry without overwriting stored intent. */
static void sj6d_tick(sj6d_lane *lane, juno6_t *s,float bpm)
{
    if(lane->present && s->typed && s->delay24.tap && bpm!=lane->applied_bpm) {
        juno6_delay_native_config(s,lane->q,bpm);
        lane->applied_bpm=bpm;
    }
}
static int sj6d_read(const sj6d_lane *l,unsigned id)
{
    if(!l->present) return -1;
    switch(id){
    case 24:return l->q[0]&1u;
    case 118:return (l->q[0]>>1)&1u;
    case 25: case 26: case 27:return l->q[id-24];
    default:return -1;
    }
}
#endif
