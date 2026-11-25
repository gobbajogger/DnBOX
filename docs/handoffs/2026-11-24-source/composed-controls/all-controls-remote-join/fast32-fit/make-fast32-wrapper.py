"""Adapt only the archived guarded read-only Chicago packing profile."""
from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
for token in ('a','e'):
    s,n=re.subn(r'(?m)^.*original_chicago_packed_'+token+r'_offset.*\n','',s)
    if n!=1: raise ValueError(token)
start=s.index("    generated=Path(b.OUT)/'gen/chicago_compact_private.h'")
end=s.index("    pair={",start)
s=s[:start]+r"""    vals={}
    for tag,n,sha in (('a',6099,'e60b242e17be038dfb34d9c9c046cbae87e88299b7eb5658beb7b0be20944a44'),('e',15629,'a907757d337ab56d9537509d1f86d8de64958bea5d33a98c442dcc2ed43535b8')):
        raw=(Path(b.OUT)/'gen'/('p_'+tag+'.h')).read_text()
        data=bytes(int(v.strip(),0) for v in re.search(r'\[\]\s*=\s*\{([^}]+)\}',raw)[1].split(',') if v.strip())
        assert len(data)==n and hashlib.sha256(data).hexdigest()==sha,tag
        vals['.rodata.p_'+tag]=data
"""+s[end:]
s=s.replace('    exact_fittab={','    exact_fittab={\n        ".rodata.s1c_iinit": (150,"cbb81dd5ecfee5c26520ccc38241c6e6bd7cd79ea2b9b50bd213c49710904a4e"),')
s=s.replace('    cold_factory={','    cold_factory={\n        ".rodata.s1_scale_ratio": (360,"fcf13207a079d2bad1fd2df5d93a296ff5625ab37b8a12dd9ce81839d74bbbc7"),\n        ".rodata.s1_scale_names": (113,"17fbf474592eee7619571810c7e7e502ab9f0dcd46de93f18d641b91b1262abc"),')
a='b.generate=lambda:None'
assert s.count(a)==1
s=s.replace(a,"b.CFLAGS.append('-DX0X_CHICAGO_FAST32=1')\n"+a)
Path(sys.argv[2]).write_text(s)
