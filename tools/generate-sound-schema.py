#!/usr/bin/env python3
"""Generate cold descriptive metadata. It does not implement DSP parameter laws."""
import argparse,pathlib,json,struct,hashlib,re,math
p=argparse.ArgumentParser();p.add_argument('--contract',type=pathlib.Path,required=True);p.add_argument('--audit',type=pathlib.Path,required=True);p.add_argument('--s1-header',type=pathlib.Path,required=True);p.add_argument('--out',type=pathlib.Path,required=True);a=p.parse_args();a.out.mkdir(parents=True,exist_ok=True)
j=json.loads(a.contract.read_text());audit=json.loads(a.audit.read_text());parameters={x['id']:x for x in audit['parameters']};defaults={x['id']:x for x in j['defaults']}
def bits(x):return struct.unpack('<I',struct.pack('<f',x))[0]
def string(x):return json.dumps(x,ensure_ascii=True)
lines=['/* Generated descriptive metadata; graph admission is a separate gate. */','static const dnbox_sj6_control descriptors[64]={']
for page in audit['pages'][:8]:
 for id in page['ids']:
  x=parameters[id];d=defaults[id];derived=d['kind']!='literal_binary32'
  if derived:
   value=math.pow((d['input_double']-d['minimum'])/(d['maximum']-d['minimum']),1/d['taper']);word=bits(value)
  else:word=int(d['bits'],16)
  lines.append('{%d,%d,0x%08xu,%s,%s,%s},'%(id,derived,word,string(x['identifier']),string(x['units']),string('; '.join(x['notes']) or 'Native stored control; no invented physical conversion.')))
lines+=['};','static const char *const page_names[8]={'+','.join(string(x['name']) for x in audit['pages'][:8])+'};']
(a.out/'sj6_controls_data.inc').write_text('\n'.join(lines)+'\n')
s=a.s1_header.read_text();enum=re.search(r'enum S1Parameter\s*\{(.*?)\};',s,re.S)
if not enum: enum=re.search(r'enum\s*\{(.*?)\};',s,re.S)
assert enum,'S1 native enum missing'
names={int(i):n for n,i in re.findall(r'\b(\w+)\s*=\s*(\d+)',enum.group(1))};assert all(i in names for i in range(150))
arrays={}
for name in ['minimums','defaults','maximums']:
 raw=re.search(r'\b'+name+r'\[150\]\s*=\s*\{([^}]*)\}',s).group(1);arrays[name]=[bits(float(x.strip().rstrip('f'))) for x in raw.split(',')];assert len(arrays[name])==150
units={15:'Hz',47:'Hz',16:'Hz',22:'seconds',23:'seconds',25:'seconds',26:'seconds',27:'seconds',29:'seconds',36:'Hz',39:'Hz',43:'seconds',64:'BPM',143:'Hz',144:'seconds'}
lines=['/* Native source physical values, not generic Drone normalized knobs. */','static const dnbox_s1_control s1_descriptors[150]={']
for i in range(150):
 flags=(1 if i in [15,47,36,43,140,147] else 0)|(2 if i in range(33,45) else 0)
 lines.append('{%d,%d,0x%08xu,0x%08xu,0x%08xu,%s,%s},'%(i,flags,arrays['minimums'][i],arrays['defaults'][i],arrays['maximums'][i],string(names[i]),string(units.get(i,'native physical value; consult original setter'))))
lines+=['};'];(a.out/'synthone_controls_data.inc').write_text('\n'.join(lines)+'\n')
(a.out/'sound_schema_provenance.json').write_text(json.dumps({'inputs':{label:{'file':path.name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest()} for label,path in [('sj6_contract',a.contract),('sj6_audit',a.audit),('synthone_native_header',a.s1_header)]},'sj6_descriptors':64,'sj6_native_count':218,'synthone_native_count':150,'derived_default_policy':'HOST_REFERENCE pow only; no claim of original Apple platform pow words. Literal defaults are exact retained binary32 words.','graph_binding':'NOT_ADMITTED; metadata/blob helpers do not alter or activate an engine'},indent=2)+'\n')
