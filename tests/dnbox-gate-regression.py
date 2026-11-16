#!/usr/bin/env python3
import pathlib,tempfile,subprocess,os
s=(pathlib.Path(__file__).resolve().parents[1]/'tools/dnbox-gate.sh').read_text()
func=s[s.index('fw_build()'):s.index('ui_defects()')];func=func[:func.index('flashmap()')]+func[func.index('budget()'):]
sha='a'*40
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d);(p/'build').mkdir();(p/'build.sh').write_text('#!/bin/sh\n[ "$MODEL" = elite ] && [ "${FAIL_ELITE:-0}" = 1 ] && exit 1\necho "XIP free: 2048"\necho "pool margin: 1024"\n');(p/'build.sh').chmod(0o755)
 prefix=f'OUT=build; SHA={sha}; XIP_MIN=1024; POOL_MIN=512;\n'+func+'\n'
 def run(code):return subprocess.run(['bash','-c',prefix+code],cwd=p,capture_output=True,text=True).returncode
 assert run('FAIL_ELITE=1; export FAIL_ELITE; fw_build')!=0
 assert run('budget')!=0
 assert run('fw_build && budget')==0
 assert run('fw_build; rm "$BUILD_RUN/pro.source"; budget')!=0
 assert run('fw_build; echo wrong > "$BUILD_RUN/elite.source"; budget')!=0
 assert run('fw_build; echo "XIP free: 100" > "$BUILD_RUN/pro.log"; budget')!=0
print('PASS 6: Elite failure, absent/stale/single-model budget rejection, exact two-model success')
