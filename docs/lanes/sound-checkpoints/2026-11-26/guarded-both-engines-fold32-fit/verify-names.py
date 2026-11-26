import subprocess,sys,tempfile,re
from pathlib import Path
dirs=[Path(a) for a in sys.argv[1:]]
def src(p):
 a=(p/'chicago_program.c').read_text()
 return a[a.index('const char *original_chicago_program_name('):a.index('int original_chicago_program_routing(')]
b,n=map(src,dirs)
assert b.split('#ifdef ORIGINAL_CHICAGO_EXPANDED_FACTORY')[1].split('#else')[0]==n.split('#ifdef ORIGINAL_CHICAGO_EXPANDED_FACTORY')[1].split('#else')[0]
h=(dirs[0]/'chicago_program_private.h').read_text()
t='\n'.join(l for l in h.splitlines() if 'original_chicago_factory_name' in l)
offset=list(map(int,re.search(r'name_offset\[81\]=\{([^}]+)\}',t)[1].split(',')))
blob=bytes(map(int,re.search(r'factory_names\[879\]=\{([^}]+)\}',t)[1].split(',')))
assert blob[-1]==0 and [0]+[q+1 for q,v in enumerate(blob[:-1]) if not v]==offset
print('Worst skipped source bytes',max(x-offset[i&~7] for i,x in enumerate(offset)),'per cold name read',flush=True)
src='''#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
'''+t+b.replace('original_chicago_program_name','base_name')+n+r"""
int main(void) {
 unsigned pass=0;
 for(unsigned i=0;i<10000;i++){
  const char *a=base_name(i),*b=original_chicago_program_name(i);
  assert(a==b);if(a)assert(!strcmp(a,b));pass++;
 }
 assert(!original_chicago_program_name((unsigned)-1));
 printf("PASS %u exact returned pointers, all 81 full names and invalid IDs; unchanged backing name bytes\n",pass+1);
}
"""
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'w.c').write_text(src)
 subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined',str(p/'w.c'),'-o',str(p/'w')],check=True)
 subprocess.run([str(p/'w')],check=True)
