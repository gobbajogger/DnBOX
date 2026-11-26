from pathlib import Path
import sys
R=Path(sys.argv[1])/'tree/firmware/src'
def edit(path,repls):
 p=R/path
 t=p.read_text()
 for a,b in repls:
  assert t.count(a)==1,(path,a,t.count(a))
  t=t.replace(a,b)
 p.write_text(t)
edit('dsp/drum909.h', [('} d9_smp_t;', '''} d9_smp_t;
enum { D9_FUTURE_SLOTS = (1024 - sizeof(d9_smp_t) - sizeof(unsigned)) / sizeof(float),
       D9_FUTURE_MAX = D9_FUTURE_SLOTS < 220 ? D9_FUTURE_SLOTS : 220 };
typedef struct {
    d9_smp_t voice;
    volatile unsigned frames;
    float pcm[D9_FUTURE_SLOTS];
} d9_future_t;'''),
('void drum909_render_st_cleared(','''void drum909_future_run(d9_future_t *, int);
void drum909_render_st_future(drum909_t *, float *, float *, float *, float *,
                              int, uint16_t, unsigned, d9_future_t *);
void drum909_render_st_cleared(''')])
edit('app/original_runtime.h',[('#include "engine_arp.h"','#include "engine_arp.h"\n#include "../dsp/drum909.h"'),
('    float metal[4][256]; /* split 808 render, before the owning bass uses this space */','''    float metal[4][256]; /* split 808 render, before the owning bass uses this space */
    struct { float prefix[4][256]; d9_future_t voice; } future;'''),
('} original_runtime_workspace;', '''} original_runtime_workspace;
_Static_assert(sizeof(original_runtime_workspace)==5120 &&
               sizeof(d9_future_t)==1024 &&
               __builtin_offsetof(original_runtime_workspace,future.voice)==4096,
               "909 future must not enlarge workspaces or overlap the 808 prefix");''')])
edit('dsp/drum909.c', [
('static void d9_render(drum909_t *d, float *dry, float *rev, float *dly, int n, uint16_t audible_tracks)', '''extern void plat_cpu2_sync(void);
void drum909_future_run(d9_future_t *f, int n)
{
    d9_bus_t b={.y=f->pcm};
    unsigned count=(unsigned)(n < f->voice.mute ? n : f->voice.mute);
    d9_render_smp(&f->voice,&b,n);
    plat_cpu2_sync();
    f->frames=count;
}
static void d9_render(drum909_t *d, float *dry, float *rev, float *dly, int n, uint16_t audible_tracks,
                      unsigned wanted, d9_future_t *future)'''),
('            d9_render_smp(s, &bus, n);','''            unsigned count=0;
            if (future && wanted==order[k]) {
                plat_cpu2_sync();
                count=future->frames;
            }
            if (count) {
                plat_cpu2_sync();
                *s=future->voice;
                bus.y=future->pcm;
                d9_emit_block(&bus,(int)count);
                bus.y=y;
            } else
                d9_render_smp(s, &bus, n);'''),
('d9_render(d, dry, rev, dly, n, 0x7FFu);','d9_render(d, dry, rev, dly, n, 0x7FFu,0,0);'),
('int n, uint16_t audible_tracks, int cleared)','int n, uint16_t audible_tracks, int cleared, unsigned v, d9_future_t *future)'),
('d9_render(d, dry_l, rev, dly, n, audible_tracks);','d9_render(d, dry_l, rev, dly, n, audible_tracks,v,future);'),
('    d9_render_st_masked(d, dry_l, dry_r, rev, dly, n, audible_tracks, 1);','''    d9_render_st_masked(d, dry_l, dry_r, rev, dly, n, audible_tracks,1,0,0);
}
void drum909_render_st_future(drum909_t *d,float *l,float *r,float *rev,float *dly,
                             int n,uint16_t mask,unsigned v,d9_future_t *future)
{
    d9_render_st_masked(d,l,r,rev,dly,n,mask,1,v,future);'''),
('    d9_render_st_masked(d, dry_l, dry_r, rev, dly, n, audible_tracks, 0);','    d9_render_st_masked(d, dry_l, dry_r, rev, dly, n, audible_tracks,0,0,0);')])
edit('app/engine.c',[
('static uint8_t eng_808_split;','''static uint8_t eng_808_split, eng_d9_voice;
static d9_future_t *eng_d9_future;'''),
('            drum909_render_st_cleared(&d909, dry_l, dry_r, rev, dly, (int)n, smc_audible_drums(solo, PART_909));',
'''            drum909_render_st_future(&d909,dry_l,dry_r,rev,dly,(int)n,
                        smc_audible_drums(solo,PART_909),eng_d9_voice,eng_d9_future);'''),
('    if (eng_808_split && !eng_808_body)\n        d808_body_run();','''    if (eng_d9_future)
        drum909_future_run(eng_d9_future,(int)(eng_job_arg & 0xffu));
    if (eng_808_split && !eng_808_body)
        d808_body_run();
    /* The selected workspace is owned by the next worker bass. 909 never
     * waits and can discard unfinished work; stage 1 releases its reader.
     * Existing c1_take_over waits/stops this worker before reclaiming it. */
    if (eng_d9_future && stage_wait(1,1))
        return;'''),
('    eng_808_body = (uint8_t)!eng_808_split;\n    eng_sync();','''    eng_808_body = (uint8_t)!eng_808_split;
    eng_d9_future=0;
    if (n<=D9_FUTURE_MAX && !(m & (1u<<JB_909)) && (m & ((1u<<JB_A)|(1u<<JB_B)))) {
        /* Prefer the more expensive cymbals; choose before worker dispatch.
         * Core 1 sees a private copy, never a pointer into the live kit. */
        for (unsigned v=DR_RD;v>=DR_CH;--v) {
            d9_smp_t *s=&d909.smp[v-DR_CH];
            if (!s->playing || s->mute<=0) continue;
            d9_future_t *f=(m&(1u<<JB_A))?&original_workspace.future.voice:&original_workspace_b.future.voice;
            f->voice=*s; f->frames=0;
            eng_d9_voice=(uint8_t)v; eng_d9_future=f;
            break;
        }
    }
    eng_sync();'''),
('        eng_808_split = 0;\n        eng_808_body = 1;','        eng_808_split = 0;\n        eng_808_body = 1;\n        eng_d9_future = 0;')])
