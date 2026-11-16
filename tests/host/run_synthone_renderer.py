#!/usr/bin/env python3
"""Render original-source pre-FX A/B using a freshly built pinned source closure.
First run direct-scheduler/run_tests.py --full-sequence with authenticated inputs.
This checks retained source bytes before reusing that run's host objects. It is
not a target/device/full-FX or independent binary-reproducibility gate.
"""
import argparse,pathlib,json,hashlib,subprocess,sys
p=argparse.ArgumentParser();p.add_argument('--source-build',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();base=a.source_build.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);repo=pathlib.Path(__file__).resolve().parents[2]
for x in json.load(open(base/'COMPILED-SOURCES.json')):
 q=base/'closure/scheduler'/x['path'];assert hashlib.sha256(q.read_bytes()).hexdigest()==x['sha256'],x['path']
cmds=json.load(open(base/'commands.json'));compile=list(next(x for x in cmds if any(str(z).endswith('/tests/source_scheduler_test.cpp') for z in x)))
i=next(i for i,z in enumerate(compile) if str(z).endswith('/tests/source_scheduler_test.cpp'));compile[i]=str(repo/'tests/host/synthone_renderer_test.cpp');compile[compile.index('-o')+1]=str(out/'adapter-test.o');compile+=['-I'+str(repo/'firmware/src/dsp')]
link=[str(out/'adapter-test.o') if z==str(base/'test.o') else z for z in cmds[-1]];link[link.index('-o')+1]=str(out/'adapter-test');(out/'commands.json').write_text(json.dumps([compile,link],indent=2)+'\n')
subprocess.run(compile,check=True);subprocess.run(link,check=True);r=subprocess.run([str(out/'adapter-test'),str(base/'public-morphs.f32'),str(base/'compact.bin'),str(out/'adapter-audio.f32le')]);sys.exit(r.returncode)
