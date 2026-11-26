from pathlib import Path
import sys,hashlib,struct,json,math,collections
o,b=map(Path,sys.argv[1:])
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
receipt={}
cumul={}
for f in (0,1):
 for a in ('dry','authored'):
  k=f'{f}-{a}'
  read=lambda x:o/(k+x)
  parts=[i for (i,) in struct.iter_unpack('<I',read('-blocks.i32').read_bytes())]
  counters=[]; elapsed=0; internal=0; prev=0.
  for l in read('.log').read_text().splitlines():
   if not l.startswith('J60CLOCK '):continue
   _,mode,hz,n,emit,mix,bound,core,post,p,q=l.split()
   mode,n,emit,mix,bound,core,post=map(int,(mode,n,emit,mix,bound,core,post))
   p=float.fromhex(p);q=float.fromhex(q)
   assert float(hz)==44100. and emit==n and p==prev and bound==core==post
   assert (mode,mix,bound)==(1,0,0) if a=='dry' else (mode==2 and mix==n and bound>0)
   prev=q; elapsed+=n; internal+=bound
   ideal=elapsed*96000/44100 if a=='authored' else 0.
   counters.append([elapsed,internal,internal-ideal,p,q])
  assert elapsed==148684 and len(counters)==len(parts)
  assert [counters[i][0]-(counters[i-1][0] if i else 0) for i in range(len(parts))]==parts
  expected=b/f'sub-range.private-output/2-{a}.private.f32'
  match=sha(read('.f32'))==sha(expected)
  if f==0 or a=='dry':assert match
  if a=='dry':assert sha(read('-in.f32'))==sha(read('-out.f32'))
  ratio=internal/elapsed
  freq=ratio*44100
  receipt[k]=dict(calls=len(parts),outer=elapsed,emitted=elapsed,post_per_channel=internal,internal_bound_input=internal,
      effective_clock_hz=freq,first=counters[:3],last=counters[-3:],partition_counts=dict(collections.Counter(parts)),
      max_abs_count_vs_ideal=max(abs(x[2]) for x in counters),parts_sha=sha(read('-blocks.i32')),
      pre_in_sha=sha(read('-in.f32')),post_sha=sha(read('-out.f32')),output_sha=sha(read('.f32')),output_bit_exact_stock_reference=match)
  cumul[k]=counters
assert cumul['0-authored']==cumul['1-authored'] and cumul['0-dry']==cumul['1-dry']
assert receipt['0-authored']['pre_in_sha']==receipt['1-authored']['pre_in_sha']
print(json.dumps(dict(receipt=receipt,partitions_and_all_counts_match_between_macro_arms=True,scope='actual compiled composed 44.1 host runtime J60 counters, not timing or hardware'),indent=2))
