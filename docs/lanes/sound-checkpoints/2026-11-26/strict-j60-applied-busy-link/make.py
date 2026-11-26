from pathlib import Path
import hashlib,sys
p=Path(sys.argv[1]); out=Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
def edit(name,pin,changes):
 b=(p/name).read_bytes();assert hashlib.sha256(b).hexdigest()==pin
 s=b.decode()
 for a,z in changes:
  assert s.count(a)==1, (name,a,s.count(a))
  s=s.replace(a,z)
 (out/name).write_text(s)
G='DNBOX_LAB_MISSED_BUSY'
def add(a,b):return a, a+f'\n#if {G}\n'+b+'\n#endif'
edit('main_fm1.c','bc7f8e724a30363a087063024d77b6163b3ebbe9d6d2abb3d644a7fe696d9846',[
 add('static uint8_t safe_mode;', '''#if !X0X_COLD_COMPANION_LAB
#error busy recorder cannot be present in an installable image
#endif
/* packed ordinal/class then elapsed us, first and worst true DMA misses */
volatile uint32_t lab_miss_busy[6] X0X_POOL __attribute__((aligned(4)));
#define lab_busy_core (lab_miss_busy+4)'''),
 add('        uint32_t half = fm1_audio_free_half();','        lab_busy_core[0]=lab_busy_core[1]=0;'),
 add('        if (fm1_audio_free_half() != half) {','''            /* Keep the exact ordinal over the supported diagnostic scope;
             * never silently wrap a packed 16-bit half. Mark incomplete above it. */
            unsigned o=x0x_dbg.halves, us=x0x_dbg.last_us;
            if (o && o<=65536u) {
                uint32_t cls=lab_busy_core[0]|lab_busy_core[1];
                uint32_t row=((o-1u)<<16)|cls;
                if (!x0x_dbg.late) { lab_miss_busy[0]=row; lab_miss_busy[1]=us; }
                if (!x0x_dbg.late || us>lab_miss_busy[3]) {
                    lab_miss_busy[2]=row; lab_miss_busy[3]=us;
                }
            } else {
                lab_miss_busy[3]=~0u;
            }''')
])
edit('engine.c','92a7d2f68d027c40306a27bf954ea7f5a736e14cc8d169a97ef9b30e56cb4f54',[
 add('static uint8_t d808_was_active;','''extern volatile uint32_t lab_miss_busy[6];
#define lab_busy_core (lab_miss_busy+4)'''),
 add('    int active = 1;','    unsigned bass_when = 0, marks = 0;'),
 add('        bass_adapt_pre(bv, (unsigned)n);','        bass_when=(unsigned)original_runtime_active(&original_bass[bv]);'),
 add('        bass_render_ok[bv] = (uint8_t)(status == ORIGINAL_RUNTIME_OK);   /* recorder: core 0, after */','''        if (status == ORIGINAL_RUNTIME_OK && (bass_when || original_runtime_active(&original_bass[bv])))
            marks=1u<<(3u+(unsigned)bv*4u+(original_bass[bv].kind&3u));
        if (status != ORIGINAL_RUNTIME_OK) marks|=1u<<13;
'''),
 ('    if (active)\n        job_cost(j, plat_ticks24() - tk - wt, n);','''    if (active)
        job_cost(j, plat_ticks24() - tk - wt, n);
#if DNBOX_LAB_MISSED_BUSY
    if (active && j==JB_909) marks|=1u;
    if (active && j==JB_808) marks|=2u;
    if (active && j==JB_BRK) marks|=4u;
    if (core && marks) marks|=1u<<11;
    lab_busy_core[core]|=marks;
#endif'''),
 ('    if (eng_808_split && !eng_808_body)\n        d808_body_run();','''    if (eng_808_split && !eng_808_body) {
        d808_body_run();
#if DNBOX_LAB_MISSED_BUSY
        lab_busy_core[1]|=1u<<12;
#endif
    }'''),
 add('        if (j == JB_BRK)\n            brk_midi_count = eng_brk_mark;               /* drop a failed remote job\'s descriptors */','        lab_busy_core[0]|=1u<<15;'),
 add('        d9_test_timeline[0]=f;','        lab_busy_core[0]|=1u<<14;')
])
