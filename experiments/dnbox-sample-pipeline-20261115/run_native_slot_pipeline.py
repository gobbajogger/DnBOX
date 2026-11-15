# SPDX-License-Identifier: GPL-3.0-only
"""Reproducible native-slot probe runner. All dependencies are explicit local CLI inputs.

Use an independently authenticated current3c41 native-host recipe (for example
the reviewed #949 preparation), authenticated current-source repository tree, and private/synthetic
case manifest. No asset is embedded in this source. No network/device operation.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,struct,subprocess,sys,zlib
PIN='3c41ef4bedf9701f549ce2752b73306e2fab1eb8'
def main():
 a=argparse.ArgumentParser()
 a.add_argument('--compile-command',type=Path,required=True)
 a.add_argument('--source-root',type=Path,required=True)
 a.add_argument('--case-manifest',type=Path,required=True)
 a.add_argument('--output-directory',type=Path,required=True)
 a.add_argument('--source-pin',choices=[PIN],required=True)
 args=a.parse_args();D=args.output_directory.resolve();D.mkdir(parents=True,exist_ok=True)
 source=Path(__file__).with_name('native-slot-pipeline.c').resolve();exe=D/'native-slot-pipeline'
 cmd=json.loads(args.compile_command.read_text())
 if not isinstance(cmd,list)or not cmd or any(not isinstance(x,str)for x in cmd):raise ValueError('trusted compile argument list required')
 replaced=0;out=False;new=[]
 for i,x in enumerate(cmd):
  if x.endswith('/source_native_probe.c'):new.append(str(source));replaced+=1
  elif x=='-o':new.append(x);out=True
  elif out:new.append(str(exe));out=False
  else:new.append(x)
 if replaced!=1 or not any(x==str(exe)for x in new):raise ValueError('admitted native recipe shape not recognized')
 host=(args.source_root.resolve()/'host/x0x_host.c')
 if not host.is_file():raise ValueError('authenticated original host source required')
 new.insert(1,'-DDNBOX_HOST_SOURCE=\"'+str(host)+'\"')
 (D/'command.json').write_text(json.dumps(new,indent=2)+'\n')
 p=subprocess.run(new,capture_output=True,text=True);(D/'compile.log').write_text(p.stdout+p.stderr)
 if p.returncode:raise RuntimeError('native compile failed; see compile.log')
 receipt=json.loads(args.case_manifest.read_text());results=[];renders=0
 for case in receipt['all32_subsets']:
  paths=['-']*3;expected=[]
  for w in case['writes']:
   path=Path(w['path']).resolve();b=path.read_bytes()
   if len(b)!=w['bytes']or hashlib.sha256(b).hexdigest()!=w['sha256']:raise ValueError('case image identity changed')
   paths[w['slot_index']]=str(path)
  for path in paths:
   if path=='-':continue
   b=Path(path).read_bytes()
   for i in range(b[6]):
    off,n=struct.unpack_from('<II',b,32+i*28);expected.append((n,zlib.crc32(b[512+off:512+off+(n+1)//2])))
  if len(expected)!=len(case['selected_ids']):raise ValueError('case asset count differs')
  probes=[]
  for target in range(len(expected))if expected else[-1]:
   p=subprocess.run([str(exe),*paths,str(target)],capture_output=True,text=True)
   (D/('case-'+str(case['mask']).zfill(2)+'-zone-'+str(target)+'.stderr')).write_text(p.stderr)
   if p.returncode:raise RuntimeError('native case/zone failed: '+str((case['mask'],target)))
   got=json.loads(p.stdout)
   if [(v['frames'],v['data_crc32'])for v in got['zones']]!=expected:raise ValueError('native metadata/encoded CRC changed')
   rendered=[i for i,v in enumerate(got['zones'])if v['rendered']]
   if rendered!=([target]if expected else[]):raise ValueError('fresh-process causal target mismatch')
   probes.append(got);renders+=len(rendered)
  results.append({'mask':case['mask'],'fresh_zone_processes':probes})
 out={'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'source_pin':PIN,'case_manifest_sha256':hashlib.sha256(args.case_manifest.read_bytes()).hexdigest(),'isolated_selected_zone_renders':renders,'cases':results,'limits':['Fresh native-host process per selected-zone target; no preceding voice/FX history.','Actual C slot scan/menu selection and finite positive engine output, not original-WAV/audio fidelity or device/target fit.','No network/device writes or public/private input transmission.']}
 (D/'RESULT.json').write_text(json.dumps(out,indent=2)+'\n')
 print(json.dumps({'created_utc':out['created_utc'],'cases':len(results),'isolated_selected_zone_renders':renders,'result_sha256':hashlib.sha256((D/'RESULT.json').read_bytes()).hexdigest()}))
if __name__=='__main__':main()

