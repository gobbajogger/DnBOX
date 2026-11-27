#!/usr/bin/env python3
from pathlib import Path
from hashlib import sha256
import sys
src,dst=map(Path,sys.argv[1:])
pins={'app/engine.c':'5e2700fdfb1ed4ab612cd9788394e7b5e1e1af1083748170fe819b8cf6039612',
      'dsp/s1c/s1c.c':'bd56daf68d91dbfffffe5be7b9d7a61db5124334a6dad8fba4742043c7a7213d'}
def prep(key,edits):
    src_data=(src/key).read_bytes()
    assert sha256(src_data).hexdigest()==pins[key],key
    s=src_data.decode()
    for a,b in edits:
        assert s.count(a)==1,(key,a,s.count(a))
        s=s.replace(a,b)
    q=dst/key;q.parent.mkdir(parents=True,exist_ok=True);q.write_text(s)
prep('app/engine.c',[
('static uint8_t eng_c1_waited;', '''#if DNBOX_S1_LATER_TILE
/* Core0 sole writer, no new wait and never touches eng_stage. */
volatile uint32_t lab_s1_later_epoch, lab_s1_later_c0_edge;
static uint32_t lab_s1_later_serial;
#if DNBOX_S1_LATER_TILE_EXECUTE
/* External adapter must use proven per-epoch private ownership/ACK.
 * Exactly one bounded claim attempt at this source edge. */
extern void s1_lab_late_try_c0_once(uint32_t epoch,unsigned edge);
extern int s1_lab_late_retire(int c1_quiesced);
#endif
static void s1_later_c0_edge(unsigned edge) {
    if(!lab_s1_later_epoch) return;
    lab_s1_later_c0_edge=edge;
    eng_sync();
#if DNBOX_S1_LATER_TILE_EXECUTE
    s1_lab_late_try_c0_once(lab_s1_later_epoch,edge);
#endif
}
#endif
static uint8_t eng_c1_waited;'''.replace('    eng_sync();','    plat_cpu2_sync();')),
('''    t0 = plat_ticks24();
    for (;;) {
        eng_sync();
        if (eng_stage >= s)''','''#if DNBOX_S1_LATER_TILE
    if(s==3u) s1_later_c0_edge(3u);
#endif
    t0 = plat_ticks24();
    for (;;) {
        eng_sync();
        if (eng_stage >= s)'''),
('''        if(s==3u && eng_c1_mask==(1u<<JB_A) && original_bass[1].kind==ORIGINAL_SOUND_CHICAGO){
            extern void s1_diagnostic_try_wet(void);
            s1_diagnostic_try_wet();
        }''','''#if !DNBOX_S1_LATER_TILE
        if(s==3u && eng_c1_mask==(1u<<JB_A) && original_bass[1].kind==ORIGINAL_SOUND_CHICAGO){
            extern void s1_diagnostic_try_wet(void);
            s1_diagnostic_try_wet();
        }
#endif'''),
('''        eng_stage = 2;
    } else if (j == JB_A''','''        eng_stage = 2;
#if DNBOX_S1_LATER_TILE
        if(!core) s1_later_c0_edge(2u);
#endif
    } else if (j == JB_A'''),
('''    eng_808_body = (uint8_t)!eng_808_split;
    eng_sync();''','''    eng_808_body = (uint8_t)!eng_808_split;
#if DNBOX_S1_LATER_TILE
    lab_s1_later_epoch=0;
    lab_s1_later_c0_edge=0;
    if(m==(1u<<JB_A) && n>=32u && original_bass[0].kind==ORIGINAL_SOUND_SYNTHONE &&
        original_bass[0].status==ORIGINAL_RUNTIME_OK &&
        original_bass[1].kind==ORIGINAL_SOUND_CHICAGO) {
        if(++lab_s1_later_serial==0) ++lab_s1_later_serial;
        lab_s1_later_epoch=lab_s1_later_serial;
    }
#endif
    eng_sync();'''),
('''    if (m && !plat_cpu2_run(core2_jobs, 0u)) {
        eng_c1_mask = 0;''','''    if (m && !plat_cpu2_run(core2_jobs, 0u)) {
#if DNBOX_S1_LATER_TILE
        lab_s1_later_epoch=0;
#endif
        eng_c1_mask = 0;'''),
('''        c1_take_over(JB_N + 1u);''','''#if DNBOX_S1_LATER_TILE
        lab_s1_later_epoch=0; /* no admission for local takeover */
#endif
        c1_take_over(JB_N + 1u);
#if DNBOX_S1_LATER_TILE && DNBOX_S1_LATER_TILE_EXECUTE
        /* C0 helper is synchronous, and native wait success or failed
         * mailbox hold stops further Core1 access. Do not retire on a
         * custom host stub whose wait does not establish quiescence. */
#ifndef X0X_HOST
        (void)s1_lab_late_retire(eng_c1_waited!=0);
#endif
#endif'''),
('''            c1_take_over(s);''','''#if DNBOX_S1_LATER_TILE
            lab_s1_later_epoch=0; /* no admission for local takeover */
#endif
            c1_take_over(s);''')])
prep('dsp/s1c/s1c.c',[
('static s1c *volatile wet_exact_owner;', '''#if DNBOX_S1_LATER_TILE
extern volatile uint32_t lab_s1_later_epoch, lab_s1_later_c0_edge;
#if !DNBOX_S1_LATER_TILE_EXECUTE
/* Sole writer is admitted A/Core1, stop capture before reading. */
volatile uint32_t lab_s1_later_counts[7];
#endif
#endif
static s1c *volatile wet_exact_owner;'''),
('''    if(wet_exact_owner==s && n>=32u && lab_wet_allow){''','''    if(!DNBOX_S1_LATER_TILE && wet_exact_owner==s && n>=32u && lab_wet_allow){'''.replace('    if(!DNBOX_S1_LATER_TILE && wet_exact_owner==s && n>=32u && lab_wet_allow){','#if !DNBOX_S1_LATER_TILE\n    if(wet_exact_owner==s && n>=32u && lab_wet_allow){')),
('''    int concurrent=0;
    for(unsigned first=0;first<n;first+=16u){''','''#endif
    int concurrent=0;
    for(unsigned first=0;first<n;first+=16u){
#if DNBOX_S1_LATER_TILE && !DNBOX_S1_LATER_TILE_EXECUTE
        if(wet_exact_owner==s && lab_s1_later_epoch){
            if(!first) ++lab_s1_later_counts[0];
            else if(n-first!=0){
                ++lab_s1_later_counts[1];
                if(n-first<16u) ++lab_s1_later_counts[2];
                else if(P(MONO)!=0.0f || s->t[MONO]!=0.0f)
                    ++lab_s1_later_counts[3];
                else {
                    unsigned mask=0;
                    for(unsigned v=0;v<S1C_VOICES;++v)
                        if(s->v[v].root!=-1 && s->v[v].stage) mask|=1u<<v;
                    if((mask&7u)==0 || (mask&~7u)==0) ++lab_s1_later_counts[4];
                    else {
                        plat_cpu2_sync();
                        ++lab_s1_later_counts[5];
                        if(lab_s1_later_c0_edge==3u) ++lab_s1_later_counts[6];
                    }
                }
            }
        }
#endif''')])
