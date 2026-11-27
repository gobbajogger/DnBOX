import json,struct,pathlib
p=pathlib.Path(__file__).resolve().parent
s=p.parent/'sj6-parity-suite'
m=json.loads((s/'private/reference-manifest.json').read_text())
assert m['preset_uid']=='1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94'
ids=[0,1,3,7,8,11,12,13,14,15,16,22,23,28,36,37,38,39,57,59,60,61,62,78,79,82,83,86,105,106,116,119,120,121,122,132,133,134,135,137,201,202,210,211]
words={k:struct.unpack('<f',bytes.fromhex(m['controls']['authored'][str(k)]))[0] for k in ids}
assert [k for k in m['controls']['authored'] if m['controls']['authored'][k]!=m['controls']['verb-off'][k]]==['11']
assert words[57]==0 and words[12] in (0.,1.)
(p/'private').mkdir(exist_ok=True)
with (p/'private/words.h').open('w') as f:
 for k,w in words.items():print(f'#define P{k} {w.hex()}f',file=f)
render=(s/'render.c').read_text().replace('#include "cue.h"','#include "cue.h"\n#include "words.h"\n#include "project.inc"')
render=render.replace('argc==2 && (!strcmp(argv[1],"0") || !strcmp(argv[1],"1"))','argc==3 && (!strcmp(argv[1],"0") || !strcmp(argv[1],"1"))')
render=render.replace(' unsigned f=0,e=0;', ''' if(strcmp(argv[2],"0")) {
   REQUIRED(!strcmp(argv[2],"1") || !strcmp(argv[2],"2") || !strcmp(argv[2],"3"));
   REQUIRED(apply_brainwave_projection(&p,wet,argv[2][0]=='2'));
   if(argv[2][0]=='3') {
      /* Diagnostic comparison at the pre-master boundary, not a proposed correction. */
      p.dsp.juno.synth.master_gain=1.f;
      fprintf(stderr,"explicit diagnostic unity master\\n");
   }
 }
 unsigned f=0,e=0;''')
(p/'private/render.c').write_text(render)
