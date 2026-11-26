from pathlib import Path
import hashlib,json,struct,subprocess,sys
elf,ram=map(Path,sys.argv[1:])
symbols={}
for line in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
 c=line.split()
 if len(c)>=8 and c[2].isdigit() and c[6]!='UND':symbols[c[7]]=(int(c[1],16)-0x01c00000,int(c[2]))
b=ram.read_bytes()
def read(name,fmt):
 a,s=symbols[name]; assert s==struct.calcsize(fmt)
 return struct.unpack_from(fmt,b,a)
half,us,ms,masks,N,*metrics=read('lab_morph_trace','<17I')
assert half>0 and us>0 and 1<=N<=6
names=('span_enter','dispatched','c1_enter','c1_after808','c0_after909','c1_done','c0_jobs_returned',
       'joined','recorder_done','mix_ready','shared_fx_published','end_span')
raw=read('lab_full','<72H')
spans=[dict(zip(names,[(v-1)/3 if v else None for v in raw[12*i:12*(i+1)]])) for i in range(N)]
for v in spans:
 assert v['span_enter'] is not None and v['joined'] is not None and v['end_span'] is not None
 v['job_gate_us']=v['joined']-v['span_enter']
 v['recorder_us']=v['recorder_done']-v['joined']
 v['post_join_mix_us']=v['mix_ready']-v['recorder_done']
 v['tail_us']=v['end_span']-v['mix_ready']
 if v['c1_done'] is not None:
  v['c1_tail_idle_before_mix_us']=max(0,v['mix_ready']-v['c1_done'])
jobgate=sum(x['job_gate_us'] for x in spans)
active=[sum(metrics[i:i+5])/24 for i in [0,5]]
post=max(0,us-jobgate)
prefix808=sum((x['c1_after808']-x['c1_enter']) for x in spans if x['c1_after808'] is not None and x['c1_enter'] is not None)
job_bound=sum(active)/2
# Prefix interval also brackets trace/dispatch bookkeeping at most few µs.
bound_with_prefix=(sum(active)+prefix808)/2
opt_bound=post+job_bound
# True owner windows and profiler distortion still constrain scheduling.
print(json.dumps(dict(elf_sha256=hashlib.sha256(elf.read_bytes()).hexdigest(),worst_half=half,worst_us=us,
 ms=ms,masks=[hex(i) for i in range(32) if masks&(1<<i)],
 spans=spans,active_measured_us_per_core=active,
 job_exec_us=[[t/24 for t in metrics[i:i+5]] for i in [0,5]],
 ordered_wait_us=[w/24 for w in metrics[10:]],
 measured_job_gate_wall_us=jobgate,other_time_not_in_job_gate_us=post,
 perfect_fractional_job_balancing_bound_us=job_bound,optimistic_fixed_tail_total_us=opt_bound,
 minimum_additional_latency_reduction_us=max(0,opt_bound-10809),
 already_split_808_body_interval_us=prefix808,
 optimistic_total_including_already_split_808_us=post+bound_with_prefix,
 additional_cut_after_perfect_balance_including_808_us=max(0,post+bound_with_prefix-10809),
 total_recorder_us=sum(v['recorder_us'] for v in spans),
 total_mix_between_jobs_and_fx_us=sum(v['post_join_mix_us'] for v in spans),
 total_tail_fx_master_us=sum(v['tail_us'] for v in spans),
 assumptions=['Instrumented, not a release equivalence check. Each core local interval includes any nested IRQ and traces.',
 'Total is an optimistic necessary bound at measured job demand if tail stays as source-order barriers currently require; no assumptions about overlapping absolute times counted twice.',
 'Unmeasured 808 body (between C1 start/after808), orchestration and mix/upload are absent from freely balanceable work, so perfect job balancing is not an achievable predicted speedup.']),indent=2))
