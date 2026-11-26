from pathlib import Path
import re,sys
d=Path(sys.argv[1]); p=d/'s1c_tables.inc'; s=p.read_text()
a,b=[list(map(int,re.search(r's1c_i'+k+r'\[150\] = {([^}]+)',s)[1].split(','))) for k in ['min','max']]
rows=list(zip(a,b)); bounds=sorted(set(rows)); ops=[bounds.index(c) for c in rows]
assert len(bounds)==43
i=s.index('static const uint8_t s1c_imin'); end=s.index('static const uint8_t s1c_iinit')
old=s[i:end]
new='static const uint8_t s1c_limits[43][2] = {'+','.join('{%d,%d}'%t for t in bounds)+'};\n'
new+='static const uint8_t s1c_limits_row[150] = {'+','.join(map(str,ops))+'};\n'
assert all(bounds[ops[i]]==rows[i] for i in range(150))
p.write_text(s[:i]+'#if DNBOX_S1_BOUND_PAIR_LAB\n'+new+'#else\n'+old+'#endif\n'+s[end:])
p=d/'s1c_front.c';s=p.read_text();a=s.index('#define MINV');b=s.index('float s1c_front_init',a);
p.write_text(s[:a]+'#if DNBOX_S1_BOUND_PAIR_LAB\n#define MINV(id) dict_f(s1c_limits[s1c_limits_row[id]][0])\n#define MAXV(id) dict_f(s1c_limits[s1c_limits_row[id]][1])\n#else\n'+s[a:b]+'#endif\n'+s[b:])
