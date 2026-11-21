#!/usr/bin/env python3
"""Existing DnBOX coefficient paths vs pinned output packets; source-free."""
import argparse,ctypes,hashlib,importlib.util,json,os,resource,struct,subprocess,sys,time
from pathlib import Path
HERE=Path(__file__).resolve().parent
REPO=HERE.parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output',type=Path,required=True,help='Fresh host evidence directory')
parser.add_argument('--write-pcm',action='store_true',help='Also retain candidate scalar streams')
args=parser.parse_args()
OUT=args.output.resolve()
OUT.mkdir(parents=True) # Prior evidence is never overwritten.
MATH=HERE.parent/'candidate_review/compact_profile_review/snapshot/fastmath.h'
if hashlib.sha256(MATH.read_bytes()).hexdigest()!='fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09':
 raise ValueError('Pinned DnBOX fastmath source changed')
METADATA={
 HERE.parent/'mapping/synthone_followup/implementer_brief/envelope_observations.json':'108f75f3965c95e1d3e2259e9d70f076bea4ca18843ec0af29e1477c0dd6d163',
 REPO/'tests/fixtures/synthone-envelope-live/observations.json':'03bbf96fd3171db6af5854241b7ec3272ecff3f44161dc2caa282070b2882390',
 REPO/'tests/fixtures/synthone-envelope-live/case_plan.json':'336e0313c6b1a4f3fb39a232f6ca74f7a53c2103cdedb225518070740dd3f4a3'}
for path,expected in METADATA.items():
 if hashlib.sha256(path.read_bytes()).hexdigest()!=expected:
  raise ValueError('Pinned observation metadata changed: '+str(path))
command=[os.environ.get('CC','cc'),'-std=c99','-O2','-shared','-fPIC','-DX0X_HOST',
 '-Wall','-Wextra','-Werror','-ffp-contract=off','-fno-fast-math',
 '-I'+str(MATH.parent),str(HERE/'coefficients.c'),'-lm','-o',str(OUT/'coefficients.so')]
build=subprocess.run(command,capture_output=True,text=True)
(OUT/'build.log').write_text(build.stdout+build.stderr)
build.check_returncode()
SPEC=importlib.util.spec_from_file_location('hypothesis', HERE.parent/'component_followup/proposed_s1_envelope.py')
P=importlib.util.module_from_spec(SPEC);SPEC.loader.exec_module(P)
LIB=ctypes.CDLL(str(OUT/'coefficients.so'))
METHODS={name:getattr(LIB,name) for name in ['coeff_fast','coeff_exp2','coeff_double_x','coeff_fixed','coeff_oracle']}
for f in METHODS.values():
 f.argtypes=[ctypes.c_float,ctypes.c_float,ctypes.c_int];f.restype=ctypes.c_float
def word(f):return struct.pack('<f',f).hex()
STATIC=HERE.parent/'mapping/synthone_followup/implementer_brief'
LIVE=REPO/'tests/fixtures/synthone-envelope-live'
started=time.monotonic()
sm=json.loads((STATIC/'envelope_observations.json').read_text());lm=json.loads((LIVE/'observations.json').read_text())
raws={'static':P.checked_packet(STATIC/'s1_envelope_observations.f32le','9b164e9b7ebf3fe95665ceaa64afb39d51442e4167305c5d7bc55e115995ade4'),
      'live':P.checked_packet(LIVE/'reference_observations.f32le','2cafbc8eb4b7aba8e22e0820ce8116d9dc7a57e6f8c41ed162c44ed98c474cfe')}
report={'comparison':'exact binary32 words, zero tolerance; no fitted gain or time alignment','input_hashes':{k:hashlib.sha256(v).hexdigest() for k,v in raws.items()},'methods':{},'coefficient_words':[]}
for name,func in METHODS.items():
 P.retention=lambda seconds,sample_rate,attack=False,func=func:func(seconds,sample_rate,attack)
 cases=[]
 for family,meta in [('static',sm),('live',lm)]:
  output=bytearray()
  for c in meta['cases']:
   n=c['frames'] if family=='static' else meta['frames_per_case']
   b=P.render(c['settings_ADSR'] if family=='static' else meta['settings_ADSR'],n,c['note_off_frame'] if family=='static' else meta['gate_off_frame'],None if family=='static' else c['stimulus'])
   offset=c['byte_offset']
   if len(b)!=n*4 or len(raws[family][offset:offset+n*4])!=n*4:
    raise ValueError('Scalar recording length mismatch')
   cases.append({'family':family,'id':c.get('case',c.get('id')),**P.compare(raws[family][offset:offset+n*4],b)})
   output.extend(b)
  if args.write_pcm:(OUT/(name+'_'+family+'.f32le')).write_bytes(output)
 report['methods'][name]={'samples':sum(x['samples'] for x in cases),'different_words':sum(x['different_words'] for x in cases),'cases':cases}
for t in [.05,.005,.2,.3,.5,.0005,.004]:
 for a in [False,True]:report['coefficient_words'].append({'seconds':P.f32(t),'attack':a,'words_le':{name:word(fn(t,44100,a)) for name,fn in METHODS.items()}})
report['edge_probes']=[]
for t in [0.,1.e-10,1./(44100*90),1./(44100*100),1./(44100*105),.0005,2.]:
 report['edge_probes'].append({'seconds':P.f32(t),'attack':False,'words_le':{name:word(fn(t,44100,False)) for name,fn in METHODS.items()}})
def mutated_retention(seconds,sample_rate,attack=False):
 value=METHODS['coeff_fast'](seconds,sample_rate,attack)
 if attack:
  bits=struct.unpack('<I',struct.pack('<f',value))[0]
  value=struct.unpack('<f',struct.pack('<I',bits+1))[0]
 return value
P.retention=mutated_retention
negative=P.render(lm['settings_ADSR'],lm['frames_per_case'],lm['gate_off_frame'],lm['cases'][0]['stimulus'])
report['negative_control']={'mutation':'one ULP higher attack retention','rejected':negative!=raws['live'][:len(negative)],**P.compare(raws['live'][:len(negative)],negative)}
report['wall_seconds']=time.monotonic()-started;report['maximum_rss_kib']=resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
report['source_hashes']={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [HERE/'candidate.h',HERE/'fixed_candidate.h',HERE/'coefficients.c',HERE/'check.py',HERE/'sweep.c',MATH,HERE.parent/'component_followup/proposed_s1_envelope.py']}
report['metadata_sha256']={str(p.relative_to(REPO)):h for p,h in METADATA.items()}
report['build_command']=command
report['build_exit']=build.returncode
report['scoped_pass']=all(m['samples']==379288 and m['different_words']==0 and all(c['finite'] for c in m['cases']) for m in report['methods'].values()) and report['negative_control']['rejected']
(OUT/'RESULT.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'methods':{n:{k:v for k,v in x.items() if k!='cases'} for n,x in report['methods'].items()},'wall_seconds':report['wall_seconds'],'maximum_rss_kib':report['maximum_rss_kib']}))

sys.exit(0 if report['scoped_pass'] else 1)
