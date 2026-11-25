from pathlib import Path
import re, struct
from hashlib import sha256
base=Path('tree/experiments/original_synth')
def pin(file,name,n,binary):
 t=(base/file).read_text()
 body=re.search(r'\b'+name+r'[^\n=]*=\s*\{([^}]+)\}',t)[1]
 pieces=[x.strip() for x in body.split(',') if x.strip()]
 if binary=='f':
    b=b''.join(struct.pack('<f',float.fromhex(s.removesuffix('f'))) for s in pieces)
 else:
    b=bytes(int(x,0) for x in pieces)
 assert len(b)==n
 return (n,sha256(b).hexdigest())
n0='original_chicago_lookup'
c0=pin('chicago_program_private.h',n0,168,'f')
n1='compact_chicago_base_codes'
c1=pin('program_values_compact.h',n1,117,'B')
p=Path('link-guarded-exact.py');s=p.read_text()
s=s.replace("    extra_fdata.update(s1_fdata)",f"""    if os.environ.get('D9_LAB_SPARE_TABLES')=='1':
        s1_fdata['.rodata.{n0}']={c0}
    extra_fdata.update(s1_fdata)""")
s=s.replace("    tables+=''.join",f"""    if os.environ.get('D9_LAB_SPARE_TABLES')=='1':
        exact_fittab['.rodata.{n1}']={c1}
    tables+=''.join""")
at="    # These are linker input sections"
s=s.replace(at,"""    if os.environ.get('D9_LAB_SPARE_TABLES')=='1':
        assert slot3_metadata
        slot3_metadata['.rodata.d9_cp_p']=96
""" + at)
s=s.replace("        assert sorted(regions)[0][1]<=sorted(regions)[1][0]","        assert all(b<=c for (_,b),(c,_) in zip(sorted(regions),sorted(regions)[1:]))")
at='    def link(header,companion,fittab,xcrc):'
s=s.replace(at,"""    def audit_fittab_map():
        if os.environ.get('D9_LAB_SPARE_TABLES')!='1':return
        mm=mp.read_text()
        rr=re.search(r'^\\.lab_fittab\\s+(0x[0-9a-f]+)\\s+(0x[0-9a-f]+)',mm,re.M)
        assert rr
        base,size=(int(v,16) for v in rr.groups())
        assert base==0x020fa000 and 32<size<=0x1000
        seen=[]
        for name,(expected,_) in exact_fittab.items():
            hits=re.findall(r'^ '+re.escape(name)+r'\\s+(0x[0-9a-f]+)\\s+(0x[0-9a-f]+)',mm,re.M)
            assert len(hits)==1
            addr,n=map(lambda v:int(v,16),hits[0])
            assert n==expected and base+32<=addr<=base+size-n
            seen.append((addr,addr+n))
            if name=='.rodata.compact_chicago_base_codes':
                print('FITTAB cold map-only',b.MODEL,name,hex(addr),n,flush=True)
        assert all(b<=c for (_,b),(c,_) in zip(sorted(seen),sorted(seen)[1:]))
        print('FITTAB span',b.MODEL,size,'remaining',0x1000-size,flush=True)
""" + at)
s=s.replace('                audit_fdata_map()','                audit_fdata_map()\n                audit_fittab_map()')
p.write_text(s)
print(n0,c0,n1,c1)
