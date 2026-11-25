"""Offline timing model for ordered DnBOX render and mix jobs.

No firmware behavior changes. Input costs must be independently measured
execution times, not wall times which already include stage/body waits.
"""
import argparse
from dataclasses import dataclass
import json

D909,D808,A,B,BREAK=range(5)

@dataclass(frozen=True)
class Job:
    render: int
    mix: int = 0

def simulate(mask, jobs, body, split=False, launch=0):
    if len(jobs)!=5 or not 0<=mask<32 or min(launch,body)<0:
        raise ValueError("bad scheduling inputs")
    if any(x.render<0 or x.mix<0 for x in jobs):
        raise ValueError("negative cost")
    if split and (not mask or mask & (1<<D808) or not body):
        raise ValueError("split needs active local 808 and a scheduled worker")
    core=[0,launch if mask else 0]
    order=[]
    mixed=0
    body_ready=0
    if split:
        body_ready=core[1]+body
        order.append(("808-body",1,core[1],body_ready,body_ready))
        core[1]=body_ready
    for j,x in enumerate(jobs):
        k=mask>>j&1
        start=core[k]
        render=x.render+(body if j==D808 and not split else 0)
        stop=start+render
        if j==BREAK:
            if x.mix:
                raise ValueError("include BREAK's final mixer separately")
            core[k]=stop
            order.append((j,k,start,stop,stop))
            continue
        ready=max(mixed,stop,body_ready if split and j==D808 else 0)
        mixed=core[k]=ready+x.mix
        order.append((j,k,start,stop,mixed))
    return max(core),order

def choose(jobs,body,allowed,active,launch=0,sticky=0,incumbent_split=False):
    eligible=allowed&active
    def valid_split(mask):
        return bool(body and active&(1<<D808) and mask and not mask&(1<<D808))
    rows=[]
    for mask in range(32):
        if mask&~eligible:continue
        for split in ((False,True) if valid_split(mask) else (False,)):
            rows.append((simulate(mask,jobs,body,split,launch)[0],mask,split))
    best=min(rows)
    if sticky & ~eligible or (incumbent_split and not valid_split(sticky)):
        return best
    current=simulate(sticky,jobs,body,incumbent_split,launch)[0]
    # Model-only: retain existing 1/8 switch requirement. Do not promote
    # the different tie order or ad-hoc split suppression to firmware.
    return best if best[0]*8<current*7 else (current,sticky,incumbent_split)

def self_test():
    # BK-808 handoff is a real predecessor even when another worker job
    # would otherwise start immediately and "mix later."
    jobs=[Job(100,5),Job(100,5),Job(100,5),Job(10,5),Job(4)]
    old=simulate(1<<B,jobs,400,False,10)
    new=simulate(1<<B,jobs,400,True,10)
    assert old[0]==720 and new[0]==525
    assert new[1][0]==("808-body",1,10,410,410)
    assert next(r for r in new[1] if r[0]==B)[2]==410
    assert next(r for r in new[1] if r[0]==D808)[4]==415
    assert simulate(0,[Job(0)]*5,0)[0]==0
    assert choose([Job(0)]*5,0,31,31)==(0,0,False)
    for m in range(32):
        for body in (0,10,400):
            try:
                total,rows=simulate(m,jobs,body,True,10)
            except ValueError:
                continue
            stages=[r[-1]for r in rows if type(r[0]) is int and r[0]<BREAK]
            assert stages==sorted(stages) and total>=stages[-1]
    print("Pass: ordered mix, real worker predecessor, split accounting, and stable ties.")

if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("fixture",nargs="?")
    args=p.parse_args()
    if not args.fixture:self_test()
    else:
        x=json.load(open(args.fixture))
        job=[Job(**j) for j in x.pop("jobs")]
        print(json.dumps(choose(job,**x)))
