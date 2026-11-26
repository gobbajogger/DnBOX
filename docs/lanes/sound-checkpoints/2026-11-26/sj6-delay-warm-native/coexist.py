"""Executable counterexamples for preserving active source while slicing."""
import json,math
CAP=1024; m=CAP-1
P=1023; s=16; W=32; ratio=W//s; fill=CAP
# Current is an eligible old live tap: 2 old cells, exactly one coarse cell.
# Such a low old time can persist in a routed->manual ownership history.
current=2.0; phase=0
assert current*s>=W and ratio<=fill
x=[(((i*7919+53)&32767)-16384,((i*1789+53)&32767)-16384) for i in range(CAP)]
def reads():
 age=max(1,current+1-(phase+1)/s); d=int(age)
 return ((P-d)&m,(P-d-1)&m),age-d
def live(z):
 (a,b),f=reads()
 return [aa+(bb-aa)*f for aa,bb in zip(z[a],z[b])]
z=x.copy()
old=live(z)
def group(src,i):
 return [math.trunc(v/ratio) for v in map(sum,zip(*(src[(P-1-i*ratio-j)&m] for j in range(ratio))))]
# The real newest-to-oldest rewrite needs only a two-group microbatch to
# touch an old, still-admitted stereo interpolation input.
for i in range(2):z[(P-i-1)&m]=group(z,i)
new=live(z);assert new!=old
# Even if we protect tap reads, a full old ring has a pending producer.
# First old-cell boundary replaces its oldest word before group512 if
# another callback resumes staged computation afterwards.
k=fill//ratio-1
before=group(x,k);assert (P-1-k*ratio-(ratio-1))&m==P
written=x.copy();written[P]=(-32000,31836)
after=group(written,k);assert before!=after
print(json.dumps(dict(geometry=dict(old_stride=s,new_stride=W,cursor=P,filled=fill,pending_producer_cell=P,phase=phase),live_read=dict(cells=reads()[0],before=old,after_newest_two_staged=new),oldest_group=dict(zero_based_index=k,before=before,after_ingress_overwrite=after)),indent=2))
