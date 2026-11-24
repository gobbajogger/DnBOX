#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Generate opt-in, diagnostic-only Korg bank bridge source files.
Do not overwrite production sources or build an installable image from this."""
from pathlib import Path
import re,sys
src=Path(sys.argv[1]); dst=Path(sys.argv[2]); dst.mkdir(parents=True,exist_ok=True)
for name in ('chicago','miami'):
 s=(src/f'{name}_program.c').read_text()
 s=s.replace('program_values_compact.h','korg_base_values.h')
 s=s.replace(f'#include "{name}_program.h"',f'#include "{name}_program.h"\n#include "korg_factory.h"',1)
 start=s.index(f'int original_{name}_program_init('); end=s.index(f'int original_{name}_program_native(',start)
 n=22 if name=='chicago' else 18; count=81 if name=='chicago' else 30; e=0 if name=='chicago' else 1
 s=s[:start]+f'''int original_{name}_program_init(original_{name}_program *p,unsigned factory){{
 if(!p||factory>={count})return 0;
 original_{name}_program next;memset(&next,0,sizeof next);
 if(!dn_korg_values({e},factory,next.macro))return 0;
 for(unsigned i=0;i<{n};i++)if(!original_{name}_program_set(&next,i,next.macro[i]))return 0;
 next.revision=0;*p=next;return 1;
}}
'''+s[end:]
 start=s.index(f'const char *original_{name}_program_name(')
 end=s.index('\n}',start)+2
 s=s[:start]+f'const char *original_{name}_program_name(unsigned factory){{return dn_korg_name({e},factory);}}'+s[end:]
 (dst/f'{name}_program.c').write_text(s)

# Retain the original immutable base values (both engines), drop only values
# exclusively referenced by the now-external factory programs.
raw=(src/'program_values_compact.h').read_text()
def array(name):
 m=re.search(r'\b'+name+r'\[[^]]+\]\s*=\s*\{(.*?)\};',raw,re.S)
 if not m:raise ValueError(name)
 return [x.strip() for x in m[1].split(',') if x.strip()]
words=array('original_program_value_words')
by_engine={n:[int(x) for x in array('compact_'+n+'_base_codes')] for n in ('chicago','miami')}
used=sorted(set(sum(by_engine.values(),[]))); remap={old:new for new,old in enumerate(used)}
header=['/* SPDX-License-Identifier: GPL-3.0-only */','/* Generated from exact base metadata; factory arrays are not referenced. */','#pragma once','#include <stdint.h>',f'__attribute__((weak)) const uint32_t original_program_value_words[{len(used)}]={{'+','.join(words[i] for i in used)+'};','static inline float compact_program_value(unsigned code){union{uint32_t u;float f;}v;v.u=original_program_value_words[code];return v.f;}']
for n,codes in by_engine.items():
 header += [f'#ifdef ORIGINAL_PROGRAM_VALUES_{n.upper()}',f'static const uint8_t compact_{n}_base_codes[{len(codes)}]={{'+','.join(str(remap[i]) for i in codes)+'};',f'static inline float compact_{n}_base_value(unsigned i){{return compact_program_value(compact_{n}_base_codes[i]);}}','#endif']
(dst/'korg_base_values.h').write_text('\n'.join(header)+'\n')
print('base-only dictionary:',len(words),'->',len(used),'words')
