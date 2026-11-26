from pathlib import Path
import sys
p=Path(sys.argv[1])
def change(f,pairs):
 b=p/f;s=b.read_text()
 for a,z in pairs:
  assert s.count(a)==1,(f,a)
  s=s.replace(a,z)
 b.write_text(s)
guard='''#if DNBOX_LAB_MISSED_COST
#if !DNBOX_LAB_MISSED_BUSY
#error cost probe needs the linked busy witness
#endif
#define LAB_BUSY_WORDS 10
#else
#define LAB_BUSY_WORDS 6
#endif
'''
change('main_fm1.c',[
 ('volatile uint32_t lab_miss_busy[6]',guard+'volatile uint32_t lab_miss_busy[LAB_BUSY_WORDS]'),
 ('        lab_busy_core[0]=lab_busy_core[1]=0;', '''        lab_busy_core[0]=lab_busy_core[1]=0;
#if DNBOX_LAB_MISSED_COST
        lab_miss_busy[6]=lab_miss_busy[7]=0;
#endif'''),
 ('                    lab_miss_busy[2]=row; lab_miss_busy[3]=us;', '''                    lab_miss_busy[2]=row; lab_miss_busy[3]=us;
#if DNBOX_LAB_MISSED_COST
                    lab_miss_busy[8]=lab_miss_busy[6];
                    lab_miss_busy[9]=lab_miss_busy[7];
#endif''')
])
change('engine.c',[
 ('extern volatile uint32_t lab_miss_busy[6];',guard+'extern volatile uint32_t lab_miss_busy[LAB_BUSY_WORDS];'),
 ('    if (active)\n        job_cost(j, plat_ticks24() - tk - wt, n);','''#if DNBOX_LAB_MISSED_BUSY && DNBOX_LAB_MISSED_COST
    if (active) {
        uint32_t elapsed=plat_ticks24() - tk - wt;
        job_cost(j, elapsed, n);
        if (!core) {
            if (j==JB_909) lab_miss_busy[6]+=elapsed;
            else if (marks&((1u<<3)|(1u<<7))) lab_miss_busy[7]+=elapsed;
        }
    }
#else
    if (active)
        job_cost(j, plat_ticks24() - tk - wt, n);
#endif''')
])
