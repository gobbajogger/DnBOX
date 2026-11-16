#!/usr/bin/env python3
import pathlib,tempfile,subprocess,os
s=(pathlib.Path(__file__).resolve().parents[1]/'tools/dnbox-gate.sh').read_text()
func=s[s.index('fw_build()'):s.index('ui_defects()')];func=func[:func.index('flashmap()')]+func[func.index('budget()'):]
sha='a'*40
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d);(p/'build').mkdir();(p/'build.sh').write_text('#!/bin/sh\n[ "$MODEL" = elite ] && [ "${FAIL_ELITE:-0}" = 1 ] && exit 1\necho "app code area 500000 B, 2048 B free"\necho "pool 334848 B of 344064"\n');(p/'build.sh').chmod(0o755)
 prefix=f'OUT=build; SHA={sha}; XIP_MIN=1024; POOL_MIN=512;\n'+func+'\n'
 def run(code):return subprocess.run(['bash','-c',prefix+code],cwd=p,capture_output=True,text=True).returncode
 assert run('FAIL_ELITE=1; export FAIL_ELITE; fw_build')!=0
 assert run('budget')!=0
 assert run('fw_build && budget')==0
 assert run('fw_build; echo \"app code area 500000 B, 2048 B free\" > \"$BUILD_RUN/pro.log\"; echo \"pool 335700 B of 344064\" >> \"$BUILD_RUN/pro.log\"; budget')!=0
 assert run('fw_build; rm "$BUILD_RUN/pro.source"; budget')!=0
 assert run('fw_build; echo wrong > "$BUILD_RUN/elite.source"; budget')!=0
 assert run('fw_build; echo "app code area 500000 B, 100 B free" > "$BUILD_RUN/pro.log"; budget')!=0
print('PASS 7: Elite failure, absent/stale/single-model budget rejection, exact two-model success')
# The scanner itself cannot enable firmware PSRAM. Real configuration paths remain scanned.
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d)
 def git(*args): return subprocess.check_output(['git',*args],cwd=p,text=True).strip()
 git('init','-q');git('config','user.name','fixture');git('config','user.email','fixture@example.invalid')
 for f in ['tools/dnbox-gate.sh','tools/config.sh','firmware/config.h','build.sh']:
  q=p/f;q.parent.mkdir(parents=True,exist_ok=True);q.write_text('# base\n')
 git('add','.');git('commit','-qm','base');base=git('rev-parse','HEAD')
 scanner=s[s.index('git diff "$BASE" HEAD -- firmware'):s.index('git ls-files -s')].split('&&')[0].strip()
 for f,added,blocked in [('tools/dnbox-gate.sh',next(line for line in s.splitlines() if 'PSRAM enabled' in line),False),('tools/config.sh','PSRAM_ENABLE=true',True),('firmware/config.h','#define PSRAM_ENABLE 1',True),('build.sh','PSRAM=1',True),('tools/dnbox-gate.sh','export PSRAM_ENABLE=1',True)]:
  git('reset','--hard',base);(p/f).write_text('# base\n'+added+'\n');git('add','.');git('commit','-qm','candidate')
  result=subprocess.run(['bash','-c',f'BASE={base}; '+scanner],cwd=p,capture_output=True)
  assert (result.returncode==0)==blocked,(f,result.returncode)
print('PASS 5: detector self-line accepted, firmware/tools/build activation detected')
