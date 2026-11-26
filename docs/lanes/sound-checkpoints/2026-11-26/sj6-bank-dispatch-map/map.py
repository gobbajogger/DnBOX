import collections,hashlib,json,pathlib,re,sys,zipfile
catalog,pack,panel,projection=map(pathlib.Path,sys.argv[1:])
names=catalog.read_text().splitlines(); index={s:i for i,s in enumerate(names)}
assert len(index)==220 and hashlib.sha256(pack.read_bytes()).hexdigest()=='ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'
p=panel.read_text(); a,b=p.split('static int rd_panel_sj6_extra',1)
native={int(k):v for k,v in re.findall(r'{(\d+),\s+(R_\w+),',a)}; native.update({k:'R_SJ6J60 conditional' for k in (215,216,217)})
native.update({k:"R_SJ6J60 Classic-conditional" for k in (42,43,44)})
native.update({int(k):v for k,v in re.findall(r'case (\d+): field=(ARP_\w+)',b)})
native.update({k:'engine_arp_step_set' for k in range(147,179)})
native={k:v for k,v in native.items() if k<220}
pr={int(k) for k in re.search(r'^mapped={(.*?)}$',projection.read_text(),re.M).group(1).split(',')}
# Projection stores raw 131/136; 131 also has actual remote filtering receiver.
# Diagnostic AMP2 is explicitly conditional, and raw aliases/destination
# values are not equivalent to applying their authored independent values.
conditional={115,119,120,121,122,180,181,184,186,188,190}
group={k:('native_ui_route' if k in native else 'conditional_diagnostic' if k in conditional and k in pr else 'diagnostic_only' if k in pr else 'unsupported') for k in range(220)}
coverage=collections.Counter(); rows=collections.defaultdict(collections.Counter); missing=collections.Counter(); patterns=collections.Counter()
with zipfile.ZipFile(pack) as z:
 for m in z.namelist():
  if not m.startswith('J6presets/') or not m.endswith('.superj6.bank.json'): continue
  for r in json.loads(z.read(m))['presets']:
   vals={}
   for item in r['parameterValues']:
    k=index[item['identifier']]; v=item['value']; assert k not in vals
    vals[k]=v; rows[k]['present']+=1; rows[k]['nonzero']+=int(bool(v));coverage[group[k]]+=1
   coverage['programs']+=1
   uncovered={k for k,v in vals.items() if group[k]!='native_ui_route'}
   missing['has_uncovered']+=bool(uncovered)
   missing['has_nonzero_uncovered']+=any(vals[k]!=0 for k in uncovered)
   missing['has_unsupported']+=any(group[k]=='unsupported' for k in uncovered)
   missing['has_nonzero_unsupported']+=any(group[k]=='unsupported' and vals[k]!=0 for k in uncovered)
   if all(k in vals for k in (7,129,8,130)):
    patterns['aliases_compared']+=1
    patterns['cutoff_disagrees']+=(vals[7]!=vals[129]);patterns['res_disagrees']+=(vals[8]!=vals[130])
   for k in (34,49,52,67,68,111,117,127,212,218,219):
    patterns[names[k]+'_nonzero']+=int(bool(vals.get(k)))
assert coverage['programs']==640
print(json.dumps(dict(coverage=coverage,program_conditions=missing,patterns=patterns,rows=[dict(id=k,name=n,scope=group[k],native_route=native.get(k),**rows[k]) for k,n in enumerate(names)]),indent=2))
