from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
head=s[:s.index('static inline float physical_tick')].replace('static float clock_2x(OTA *s,float x,','static float clock_2x(float pole[4],float *cond,float x,')
head=head.replace('s->condition','(*cond)').replace('s->pole','pole')
b=Path('sj6-ota-history/preflight/split.py').read_text().split('typedef struct { Expansion up; float condition; Contraction right; } GapVoice;',1)[1].split("'''",1)[0]
b='typedef struct { Expansion up; float condition; Contraction right; } GapVoice;'+b
b=b.replace('coeff,params[0],params[1],params[2],params[3]','coeff,params')
Path(sys.argv[2]).write_text(head+b)
