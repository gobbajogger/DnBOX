from pathlib import Path
import re,sys
base,after=map(Path,sys.argv[1:])
def words(p):
 s=p.read_text()
 def scope(marker,ending):
  a=s.rindex(marker); b=s.index(ending,a)
  return s[a:b]
 labels=dict(re.findall(r'const char \*const (\w+)\[\] ?= ?(\{[^;]+\});',s))
 def norm(li):return re.sub(r'\s+','',li)
 a=scope('static const x0x_param_t *sj6_knob_desc','\n}')
 d=re.findall(r'\{"[^"\n]+",[^{}\n]+\}',a)
 # Compare every literal descriptor; reconstruct pointed-to text, not address.
 d=[norm(re.sub(r',\s*(\w+)(?=\})',lambda m:','+labels.get(m[1],m[1]),v)) for v in d]
 d=[x for x in d if x not in ('{"OFF","ON"}','{"FREE","SYNC"}')]
 arp=scope('static const x0x_param_t *arp_desc','\n}')
 seq=set(map(norm,re.findall(r'seq_enable=(\{[^}]+\});',arp)))
 mix=scope('    case V_MIX:', '\n    case ')
 remaster=re.findall(r'\{PR\(R_ENG, T_FX, 0, FX_DRIVE\), PR\(R_ENG, T_MST, 0, MST_COMP1\), PR\(R_ENG, T_MST, 0, MST_PUMP\), PR\(R_ENG, T_MST, 0, MST_DJF\)\}',mix)
 assert remaster and re.search(r'add_page_refs\(emit, PT_MASTER, (?:master_)?refs, 0, 0\);',mix)
 assert re.search(r'add_page_refs\(emit, PT_WAVE, (?:master_)?refs, 0, 0\);',mix)
 return d,seq,remaster[0]
before,post=words(base),words(after)
assert before==post,([d for d in before[0] if d not in post[0]],[d for d in post[0] if d not in before[0]])
print(f'{len(before[0])} identical literal SJ6 descriptors (text, detent/default, pointed enum strings), identical shared arp and Master/Wave page maps.')
