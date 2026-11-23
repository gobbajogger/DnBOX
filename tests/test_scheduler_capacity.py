# SPDX-License-Identifier: GPL-3.0-only
import pathlib,subprocess,tempfile,unittest
HERE=pathlib.Path(__file__).resolve().parent
class Scheduler(unittest.TestCase):
 def classify_envelopes(self,source):
  # Execute the actual shell classifier: a pattern hidden in a comment must
  # not count as a serialized generator owner merely because text exists.
  start=source.index('test_class() {');end=source.index('\n}',start)+2
  names=['ui-808-stage-rows','chicago-adsr-graph','ui-adsr-engine-graph',
         'ui-drum-amp-graph','ui-drum-single-lane','ui-settings-sections']
  script=source[start:end]+'\n'+''.join('test_class '+n+'; printf "%s %s\\n" '+n+' "$TEST_CLASS"\n' for n in names)
  result=subprocess.run(['sh','-c',script],text=True,capture_output=True,timeout=5)
  self.assertEqual(result.returncode,0,result.stderr)
  return dict(line.split() for line in result.stdout.splitlines())
 def test_envelope_generators_stay_serialized(self):
  actual=self.classify_envelopes((HERE/'run_tests.sh').read_text())
  self.assertEqual(actual,dict.fromkeys(('ui-808-stage-rows','chicago-adsr-graph',
      'ui-adsr-engine-graph','ui-drum-amp-graph','ui-drum-single-lane','ui-settings-sections'),'G'))
 def test_commented_pattern_mutant_is_parallel(self):
  source=(HERE/'run_tests.sh').read_text();anchor='\n    chicago-adsr-graph|'
  self.assertEqual(source.count(anchor),1)
  actual=self.classify_envelopes(source.replace(anchor,'    chicago-adsr-graph|'))
  self.assertEqual(actual['ui-808-stage-rows'],'G')
  for name in actual:
   if name!='ui-808-stage-rows':self.assertEqual(actual[name],'P')
 def run_case(self, mutant=False, gen_fail=False):
  source=(HERE/'run_tests.sh').read_text()
  start=source.index('\nrun() {')+1
  end=source.index('\nnote() {',start)
  run=source[start:end]
  if mutant:
   # Historical bug: take a slot before waiting for the writer lock.
   # A writer waiting behind host-build then starves unrelated tests.
   writer='[ "$name" = host-build ] || [ "$name" = gen-nonfont-tables ]'
   acquire='    [ "$cls" = W ] || '+writer+' || [ $gw = 1 ] || read -r _ <&3'
   locked='            gen_lock\n            read -r _ <&3'
   self.assertEqual(run.count(acquire),1)
   self.assertEqual(run.count(locked),1)
   run=run.replace(acquire, '    [ "$cls" = W ] || [ $gw = 1 ] || read -r _ <&3')
   run=run.replace(locked, '            gen_lock')
  with tempfile.TemporaryDirectory() as d:
   script='''set -eu
OUT=out;RES=out/results;mkdir -p "$RES" "$OUT/iso";mkfifo tokens;exec 3<>tokens
printf 'x\nx\n' >&3
X0X_PYTHONUSERBASE="$PWD"; X0X_TESTS=""; GEN_FAILED=0; JOBS=2; PHASE=1; T_SEQ=0
needs_gen(){ [ "$1" = stereo ]; }
gen_wait(){ needs_gen "$1"; }
test_class(){ case "$1" in host-build|gen-nonfont-tables|ordinary-g|stereo) TEST_CLASS=G;; *) TEST_CLASS=P;; esac; }
gen_lock(){
 echo "attempt:$name" >>events
 until mkdir glock 2>/dev/null;do sleep .01;done
}
gen_unlock(){ rmdir glock; }
run_one(){
 echo "start:$2" >>events
 if [ "$2" = host-build ];then
  while [ ! -e release-writer ];do sleep .01;done
  # Give already runnable readers a generous window before the first
  # owner returns its token; the mutant has no token to run them here.
  sleep 1
 fi
 echo "end:$2" >>events
}
'''+run+'\n'
   if gen_fail:
    script+='touch out/iso/.gen-fail\nrun stereo ignored\nrun parallel ignored\n'
   else:
    script+='''run host-build ignored
while ! grep -q '^start:host-build$' events 2>/dev/null;do sleep .01;done
run gen-nonfont-tables ignored
while ! grep -q '^attempt:gen-nonfont-tables$' events;do sleep .01;done
touch release-writer
run parallel ignored
run ordinary-g ignored
'''
   script+='wait\nread -r a <&3; read -r b <&3\n'
   r=subprocess.run(['sh','-c',script],cwd=d,capture_output=True,timeout=15)
   self.assertEqual(r.returncode,0,r.stderr)
   if gen_fail:
    results=pathlib.Path(d)/'out/results'
    self.assertIn('FAIL stereo (gen-nonfont failed)',(results/'0001').read_text())
    self.assertTrue((results/'0001.fail').is_file())
   return (pathlib.Path(d)/'events').read_text().splitlines()
 def test_parallel_not_starved(self):
  e=self.run_case()
  self.assertLess(e.index('start:parallel'),e.index('end:host-build'))
  self.assertLess(e.index('start:ordinary-g'),e.index('end:host-build'))
  self.assertLess(e.index('end:host-build'),e.index('start:gen-nonfont-tables'))
 def test_old_order_mutant_demonstrates_starvation(self):
  e=self.run_case(True)
  self.assertGreater(e.index('start:parallel'),e.index('end:host-build'))
 def test_generation_failure_no_token_leak(self):
  self.assertEqual(self.run_case(gen_fail=True),['start:parallel','end:parallel'])

if __name__=='__main__':unittest.main()
