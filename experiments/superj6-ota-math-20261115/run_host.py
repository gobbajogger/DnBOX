from pathlib import Path
import subprocess,json,os,datetime,hashlib
R=Path(__file__).resolve().parent;B=R/'build';env={'PATH':os.defpath,'LC_ALL':'C','LANG':'C','TMPDIR':str(B),'UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'};records=[];pins={str(p.relative_to(R)):hashlib.sha256(p.read_bytes()).hexdigest()for d in['candidate','baseline','math','tests']for p in(R/d).rglob('*')if p.is_file()}
for cfg,opts in[('O2',['-O2']),('Os',['-Os']),('san',['-O1','-g','-fsanitize=address,undefined,float-cast-overflow','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:
 flags=opts+['-ffp-contract=off','-fno-fast-math','-frounding-math','-I'+str(R/'math')];o=B/('numeric-'+cfg+'.o');cmd=['cc','-std=c11',*flags,'-c',str(R/'math/numeric_boundary.c'),'-o',str(o)];q=subprocess.run(cmd,capture_output=True,text=True,env=env);(B/('numeric-'+cfg+'.compile.log')).write_text(q.stdout+q.stderr);assert q.returncode==0,q.stderr
 for test,compiler,source in[('conversion','cc',R/'tests/conversion.c'),('ota','c++',R/'tests/ota_equivalence.cpp'),('fma-corners','cc',R/'accepted-fma/tests/corners.c')]:
  dest=B/(test+'-'+cfg);cmd=[compiler,'-std=c11'if compiler=='cc'else'-std=c++17',*flags,str(source),str(o),'-lm','-pthread','-o',str(dest)];q=subprocess.run(cmd,capture_output=True,text=True,env=env);assert q.returncode==0,q.stderr
  default=None
  if cfg=='san':
   d=subprocess.run([str(dest)],capture_output=True,text=True,env=env);(B/(test+'-'+cfg+'-default.log')).write_text(d.stdout+d.stderr);default=d.returncode
  ee=dict(env)
  if cfg=='san':ee['ASAN_OPTIONS']='detect_leaks=0:halt_on_error=1'
  q=subprocess.run([str(dest)],capture_output=True,text=True,env=ee);(B/(test+'-'+cfg+'.log')).write_text(q.stdout+q.stderr);r=dict(configuration=cfg,test=test,command=cmd,run=[str(dest)],default_sanitizer_exit=default,exit=q.returncode,stdout=q.stdout,stderr=q.stderr);records.append(r);print(cfg,test,q.returncode,q.stdout.strip(),flush=True);assert q.returncode==0,q.stderr
for p,h in pins.items():assert hashlib.sha256((R/p).read_bytes()).hexdigest()==h,p
(B/'HOST-RESULT.private.json').write_text(json.dumps(dict(completed_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_inputs=pins,runs=records,scope='Host execution of copied accepted OTA and own integrated variant/math only; no original binary/native execution'),indent=2)+'\n')
