#!/usr/bin/env python3
"""Host timing and counter audit of a frozen candidate; no native WCET claim."""
import argparse,datetime,hashlib,json,pathlib,platform,re,subprocess
ROOT=pathlib.Path(__file__).resolve().parent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--label',required=True,help='New evidence directory; existing results are never overwritten')
args=parser.parse_args()
if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._-]*',args.label):parser.error('label must be a simple new directory name')
OUT=ROOT/args.label
try:
    OUT.mkdir()
except FileExistsError:
    parser.error('evidence directory already exists; select a new label')
SRC=ROOT/'frozen_02/snapshot'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
commands=[]
def run(cmd,stem):
    cp=subprocess.run(cmd,text=True,capture_output=True)
    commands.append({'argv':cmd,'exit_code':cp.returncode})
    (OUT/'COMMANDS.json').write_text(json.dumps(commands,indent=2)+'\n')
    (OUT/(stem+'.stdout')).write_text(cp.stdout);(OUT/(stem+'.stderr')).write_text(cp.stderr)
    cp.check_returncode();return cp.stdout

flags=['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-ffp-contract=off','-DX0X_HOST','-I'+str(SRC)]
run(flags+[str(ROOT/'callback_probe.c'),str(SRC/'dnbx_core.c'),'-o',str(OUT/'timing_probe')],'timing_build')
timing=json.loads(run([str(OUT/'timing_probe')],'timing_run'))

# A separate instrumented COPY counts all existing COUNT sites even when
# the public dnbx_work argument is NULL. Direct transcendental calls are
# independently intercepted to include pitch increments/set_layer work.
# None of these transformations is used for timing or deployed code.
original=(SRC/'dnbx_core.c').read_text()
injection='''\nstatic dnbx_work review_total_work;\nstatic uint64_t review_math[2];
static float review_exp2(float x) { review_math[0]++; return fm_exp2f(x); }
static float review_log2(float x) { review_math[1]++; return fm_log2f(x); }
#define fm_exp2f(x) review_exp2(x)
#define fm_log2f(x) review_log2(x)
void review_reset(void) { memset(&review_total_work,0,sizeof review_total_work); memset(review_math,0,sizeof review_math); }
void review_get_trace(dnbx_work *w,uint64_t *m) { *w=review_total_work; m[0]=review_math[0];m[1]=review_math[1]; }
'''
count='#define COUNT(w, member, value) do { if (w) (w)->member += (value); } while (0)'
replacement='#define COUNT(w, member, value) do { review_total_work.member += (value); if (w) (w)->member += (value); } while (0)'
assert original.count('#include <string.h>')==1 and original.count(count)==1
traced=original.replace('#include <string.h>','#include <string.h>\n'+injection).replace(count,replacement)
(OUT/'dnbx_core_trace.c').write_text(traced)
run(flags+['-DREVIEW_TRACE',str(ROOT/'callback_probe.c'),str(OUT/'dnbx_core_trace.c'),'-o',str(OUT/'counter_probe')],'counter_build')
trace=json.loads(run([str(OUT/'counter_probe')],'counter_run'))
trace['unreported_event_count_sites']={k:trace['trace_all_count_sites'][k]-trace['public_render_counters'][k] for k in trace['public_render_counters'] if k!='maximum_transition_chain'}
trace['unreported_direct_exp2_calls']=trace['trace_direct_math']['exp2_calls']-trace['public_render_counters']['exp2_calls']
trace['unreported_direct_log2_calls']=trace['trace_direct_math']['log2_calls']-trace['public_render_counters']['log2_calls']
trace['instrumentation_limit']='Counter macros and direct exp2/log2 calls only; no native instruction, cycle, branch, memory or complete math-operation instrumentation. maximum_transition_chain is updated outside COUNT, so total trace field is not meaningful.'
cpu=subprocess.run(['lscpu'],capture_output=True,text=True)
result={'created_at_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'candidate_sha256':{p.name:sha(p) for p in SRC.iterdir()},'probe_sha256':sha(ROOT/'callback_probe.c'),'runner_sha256':sha(pathlib.Path(__file__)),'trace_copy_sha256':sha(OUT/'dnbx_core_trace.c'),'compiler':subprocess.check_output(['cc','--version'],text=True).splitlines()[0],'host':{'uname':platform.uname()._asdict(),'cpu_info':cpu.stdout,'cpu_query_exit':cpu.returncode,'cpu_query_error':cpu.stderr},'burst_per_selected_callback':{'note_on':16,'note_off':16,'layer_parameter_updates':4,'raw_control_stores':436,'sustain_updates':4,'velocity_mode_updates':2},'timing':timing,'counter_audit':trace,'qualification':['Observed Linux host wall-time distribution, not native worst-case execution time. OS scheduling, frequency, cache and thermal effects are not controlled.','512 selected frames per part are processed as two256-frame calls per part, matching the native half/chunk quantum only; this is not full firmware audio_half_render.','Two selected candidate parts and the fixed listed burst are inside the timed region. Other instruments, global/private effects, output/UAC mixing, interrupt interference and event decoding are not included.','The event API has no queue or event cap. A full callback cost cannot be bounded unless the caller caps event/control delivery per callback. The listed burst is a test workload, not such a production contract.']}
(OUT/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');(OUT/'COMMANDS.json').write_text(json.dumps(commands,indent=2)+'\n')
print(json.dumps({'cases':[{k:c[k] for k in ('scenario','frames_per_part','counters_enabled','median_ns','p99_ns','observed_max_ns')} for c in timing['cases']],'counter_audit':trace},indent=2))
