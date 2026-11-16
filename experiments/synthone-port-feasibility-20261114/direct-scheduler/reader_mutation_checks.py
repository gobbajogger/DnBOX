#!/usr/bin/env python3
"""Compile/link controlled runtime-only reader faults against an exact completed run.

Audio faults use scheduled-only scenarios and require a finite/nonzero ORIGINAL
PCM witness before the runtime assertion fails. Admission/lifetime faults are
separately labeled. Compile errors are never counted as detection.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,tempfile
from datetime import datetime,timezone

def need(x,message):
 if not x:raise SystemExit(message)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--output-root',type=Path,required=True);a=p.parse_args();B=a.build.resolve();root=Path(__file__).resolve().parent;local=B/'closure/scheduler';commands=json.loads((B/'commands.json').read_text());records=json.loads((B/'COMPILED-SOURCES.json').read_text());a.output_root.mkdir(parents=True,exist_ok=True);O=Path(tempfile.mkdtemp(prefix='reader-mutations.',dir=a.output_root.resolve()));env={'PATH':os.defpath,'LC_ALL':'C','LANG':'C'}
 for e in records:
  q=local/e['path'];need(q.stat().st_size==e['bytes']and sha(q)==e['sha256'],'completed local source changed');need(q.read_bytes()==(root/e['path']).read_bytes(),'mutations require current source closure: '+e['path'])
 unit=root/'tests/direct_reader_test.c';source_manifest=json.loads((root/'SOURCE-MANIFEST.json').read_text());unit_record=next(e for e in source_manifest['files']if e['path']=='tests/direct_reader_test.c');need(unit.stat().st_size==unit_record['bytes']and sha(unit)==unit_record['sha256'],'admission unit identity mismatch')
 table=B/'public-morphs.f32';need(table.stat().st_size==852176 and sha(table)=='0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5','wrong corpus')
 base_test=(local/'tests/reader_scheduler_test.cpp').read_text();scheduled=base_test.replace('reader_admission(p,rate);','(void)&reader_admission;').replace('oscillator_boundaries(p);','(void)&oscillator_boundaries;')
 marker='CHECK(st.values.size()==n+1&&ot.values.size()==n+1);';need(scheduled.count(marker)==1,'scheduled witness site')
 witness='''bool original_finite=true,original_audible=false;for(unsigned i=0;i<n;++i){original_finite=original_finite&&std::isfinite(ol[offset+i])&&std::isfinite(orr[offset+i]);original_audible=original_audible||ol[offset+i]!=0||orr[offset+i]!=0;}CHECK(original_finite);if(original_audible)std::fprintf(stderr,"READER-MUTATION-WITNESS original_finite=1 original_audible=1\\n");'''
 scheduled=scheduled.replace(marker,witness+marker)
 specs=[
  ('wrong-row48','scheduled-audio','reader/direct_reader.c','row==48?44','row==48?48','runtime/assertion'),
  ('wrong-row49','scheduled-audio','reader/direct_reader.c','row==49?45','row==49?49','runtime/assertion'),
  ('wrong-next-index','scheduled-audio','reader/morph_compute.c','(unsigned)((pos+1)%ftp1->size)','(unsigned)pos','runtime/assertion'),
  ('numeric-not-bit-copy','scheduled-audio','reader/morph_compute.c','memcpy(&f11,&w11,4);memcpy(&f21,&w21,4);memcpy(&f12,&w12,4);memcpy(&f22,&w22,4);','f11=(SPFLOAT)w11;f21=(SPFLOAT)w21;f12=(SPFLOAT)w12;f22=(SPFLOAT)w22;','runtime/assertion'),
  ('reader-dispatch-bypassed','scheduled-path','scheduler.cpp','if(reader_active_) { if(!s1scheduler_reader::run_note','if(false && reader_active_) { if(!s1scheduler_reader::run_note','scheduled_releasing_alias_calls'),
  ('shortened-owner-span','admission-owner','scheduler.cpp','return valid_output_span(l,r,n,offset,self);','return valid_output_span(l,r,n,offset,{self.data,sizeof(State)});','valid_output(late'),
  ('raw-reinit-keeps-reader','admission-lifetime','scheduler.cpp','reader_={};reader_active_=false;','/* intentionally omitted mode retirement */','!p.s.using_reader()')]
 results=[];started=datetime.now(timezone.utc).isoformat()
 for name,kind,path,old,new,reason in specs:
  M=O/name;M.mkdir();shutil.copytree(local,M/'source');text=(M/'source'/path).read_text();need(old in text,'fault anchor missing: '+name);text=text.replace(old,new);source=M/'source'/path;source.write_text(text)
  test=M/'source/tests/reader_scheduler_test.cpp'
  if kind.startswith('scheduled'):test.write_text(scheduled)
  changed=[]
  for item in [path,'tests/reader_scheduler_test.cpp']:
   original=local/item;need(original.is_file(),'missing original unit');c=next((list(x)for x in commands if '-c'in x and x[x.index('-c')+1]==str(original)),None);need(c is not None,'missing compile command: '+item);need(Path(c[0]).name in ('cc','c++','gcc','g++','clang','clang++'),'unexpected compiler')
   obj=M/(Path(item).stem+'.o');c[c.index('-c')+1]=str(M/'source'/item);c[c.index('-o')+1]=str(obj);q=subprocess.run(c,capture_output=True,text=True,env=env);(M/(Path(item).stem+'.compile.stdout')).write_text(q.stdout);(M/(Path(item).stem+'.compile.stderr')).write_text(q.stderr);need(q.returncode==0,'mutation must compile: '+name+' '+item);changed.append((commands[next(i for i,x in enumerate(commands)if '-c'in x and x[x.index('-c')+1]==str(original))][-1],str(obj)))
  link=list(commands[-1]);need('-c'not in link,'missing link command')
  for before,after in changed:need(before in link,'object not in exact link');link=[after if x==before else x for x in link]
  binary=M/'scheduler_test';link[link.index('-o')+1]=str(binary);q=subprocess.run(link,capture_output=True,text=True,env=env);(M/'link.stdout').write_text(q.stdout);(M/'link.stderr').write_text(q.stderr);need(q.returncode==0,'mutation must link: '+name)
  q=subprocess.run([str(binary),str(table)],capture_output=True,text=True,env=env);(M/'stdout').write_text(q.stdout);(M/'stderr').write_text(q.stderr);need(q.returncode==1 and 'FAIL 'in q.stderr,'attributable runtime failure required: '+name)
  audio='READER-MUTATION-WITNESS original_finite=1 original_audible=1'in q.stderr
  if kind.startswith('scheduled'):need(audio,'finite/audible original scheduled PCM witness required: '+name)
  if reason!='runtime/assertion':need(reason in q.stderr,'wrong runtime assertion: '+name)
  results.append({'name':name,'kind':kind,'compile':0,'link':0,'runtime_exit':q.returncode,'original_finite_audible_witness':audio,'expected_assertion':reason,'source_sha256':sha(source),'stdout_sha256':sha(M/'stdout'),'stderr_sha256':sha(M/'stderr')});print(name,'DETECTED',flush=True)
 # The direct binding is also an independent admission boundary. These
 # malformed-input mutants do not claim a musical PCM witness.
 admission=[('last-row-not-scanned','(size_t)S1_MORPH_UNIQUE_ROWS*S1_MORPH_WORDS_PER_ROW','(size_t)(S1_MORPH_UNIQUE_ROWS-1)*S1_MORPH_WORDS_PER_ROW'),('negative-zero-guard-admitted','||(i%S1_MORPH_WORDS_PER_ROW==4096&&bits!=0)',''),('bad-sample-index-admitted','sample>=S1_MORPH_WORDS_PER_ROW||','0||')]
 for name,old,new in admission:
  M=O/name;M.mkdir();shutil.copytree(local/'reader',M/'reader');source=M/'reader/direct_reader.c';text=source.read_text();need(text.count(old)==1,'admission fault anchor');source.write_text(text.replace(old,new));base=next(list(c)for c in commands if '-c'in c and c[c.index('-c')+1]==str(local/'reader/direct_reader.c'));flags=base[1:base.index('-c')];need(Path(base[0]).name in ('cc','gcc','clang'),'unexpected C compiler');built=[]
  for src,obj in [(source,M/'reader.o'),(unit,M/'test.o')]:
   c=[base[0]]+flags+['-c',str(src),'-o',str(obj)];q=subprocess.run(c,capture_output=True,text=True,env=env);(M/(obj.stem+'.compile.stdout')).write_text(q.stdout);(M/(obj.stem+'.compile.stderr')).write_text(q.stderr);need(q.returncode==0,'admission mutation must compile: '+name);built.append(obj)
  binary=M/'direct_reader_test';link=[base[0]]+flags+list(map(str,built))+['-lm','-o',str(binary)];q=subprocess.run(link,capture_output=True,text=True,env=env);(M/'link.stdout').write_text(q.stdout);(M/'link.stderr').write_text(q.stderr);need(q.returncode==0,'admission mutation must link: '+name);q=subprocess.run([str(binary),str(table)],capture_output=True,text=True,env=env);(M/'stdout').write_text(q.stdout);(M/'stderr').write_text(q.stderr);need(q.returncode==1 and 'FAIL direct reader' in q.stderr,'admission runtime assertion required: '+name);results.append({'name':name,'kind':'direct-admission-unit','compile':0,'link':0,'runtime_exit':1,'original_finite_audible_witness':False,'expected_assertion':'direct reader admission/read rejection','source_sha256':sha(source),'stdout_sha256':sha(M/'stdout'),'stderr_sha256':sha(M/'stderr')});print(name,'DETECTED',flush=True)
 result={'scope':'Controlled runtime reader faults; no target/hardware proof','started_utc':started,'completed_utc':datetime.now(timezone.utc).isoformat(),'base_compiled_sources_sha256':sha(B/'COMPILED-SOURCES.json'),'admission_unit_source_sha256':sha(unit),'cases':results,'failures':0};(O/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'detected':len(results),'result_directory':str(O)}))
if __name__=='__main__':main()
