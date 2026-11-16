"""Original-source-derived, isolated control-plane reference. No DSP backend.
Cold serialized API; immutable records/raw bytes are preserved. All OK statuses
name their bounded scope. This is not a full original preset loader.
"""
from dataclasses import dataclass,replace
from pathlib import Path
import json,math,struct
import numpy as np
F=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
def fadd(a,b):return float(np.float32(a)+np.float32(b))
def fsub(a,b):return float(np.float32(a)-np.float32(b))
def fmul(a,b):return float(np.float32(a)*np.float32(b))
def fdiv(a,b):return float(np.float32(a)/np.float32(b))
def _finite(x):
    try:return type(x)in (int,float) and math.isfinite(x)
    except OverflowError:return False
def _u64(x):return type(x)is int and 0<=x<1<<64
def _finite32(x):return _finite(x) and abs(x)<=float.fromhex('0x1.fffffep127')
EPS=struct.unpack('<f',struct.pack('<I',0x3a03126f))[0]
_DATA=json.loads(Path(__file__).with_name('control_contract.json').read_text())
ORDER=tuple((x['id'],x['identifier']) for x in _DATA['order'])
DEFAULTS=tuple(_DATA['defaults'])
@dataclass(frozen=True)
class Record:
    identifier: str
    value: float
@dataclass(frozen=True)
class Command:
    op: str
    domain: str
    receiver: str
    id: int
    value: float
    generation: int
    order: int
    delay_seconds: float=0.
    token: int=0
@dataclass(frozen=True)
class Result:
    status: str
    commands: tuple=()
    records: tuple=()
    raw: bytes=b''

def _default(i,pow_fn):
    d=DEFAULTS[i]
    if d['kind']=='literal_binary32':
        return struct.unpack('<f',struct.pack('<I',int(d['bits'],16)))[0]
    if pow_fn is None:raise LookupError('MISSING_POW')
    ratio=fdiv(fsub(F(d['input_double']),F(d['minimum'])),fsub(F(d['maximum']),F(d['minimum'])))
    value=pow_fn(ratio,fdiv(1.,F(d['taper'])))
    if not _finite(value) or not 0.<=value<=1.:raise ValueError('invalid provider result')
    return F(value)

def plan_main_pass(records,raw,generation,caller_immediate,capacity,capabilities,pow_fn=None):
    """Plan ONLY26926c's main control pass, not post-LFO/tuning/whole transaction.
    Executor capabilities declare command support, not sound fidelity. pow_fn must
    be a pure finite deterministic binary32 provider; its platform identity is external.
    Capacity/validation failure returns no commands. No caller state is mutated.
    Unknown records remain intact and are not automatically DSP-supported.
    """
    if (type(records)is not tuple or type(raw)is not bytes or type(generation)is not int
        or not 0<=generation<1<<64 or len(records)>4096 or type(caller_immediate)is not bool
        or type(capacity)is not int or capacity<0 or type(capabilities)is not frozenset):
        return Result('INVALID')
    for r in records:
        if (type(r)is not Record or type(r.identifier)is not str or type(r.value)not in (int,float)
            or not _finite32(r.value)):
            return Result('INVALID')
    selected=[]
    try:
        for i,name in ORDER:
            if i in (212,213,214):continue
            r=next((r for r in records if r.identifier==name),None)
            value=F(r.value) if r is not None else _default(i,pow_fn)
            if not math.isfinite(value):return Result('INVALID')
            if i==24 and value>0:selected.append(('delay_enable_prepare',i,value,0.))
            selected.append(('defer_volume' if i==0 and not caller_immediate else 'apply_control',i,value,F(.15) if i==0 and not caller_immediate else 0.))
    except LookupError:return Result('MISSING_POW')
    except (ValueError,OverflowError,TypeError):return Result('MATH_FAILURE')
    if len(selected)>capacity:return Result('CAPACITY')
    required={'control_apply'}
    if any(x[0]=='delay_enable_prepare'for x in selected):required.add('delay_enable_prepare')
    if any(x[0]=='defer_volume'for x in selected):required.add('deferred_volume')
    if not required<=capabilities:return Result('MISSING_CAPABILITY')
    commands=tuple(Command(op,'normalized_control','instrument_control',i,v,generation,n,delay)for n,(op,i,v,delay)in enumerate(selected))
    return Result('OK_MAIN_PASS_ONLY',commands,records,raw)

@dataclass(frozen=True)
class CutoffState:
    generation: int
    current: float
    target: float
    active: bool
    token: int=0
    awaiting: bool=False
@dataclass(frozen=True)
class Transition:
    status: str
    state: object
    commands: tuple=()
def cutoff_step(state,dt,capacity,capabilities,exp_fn=None):
    # Caller serializes this state; command acceptance is not DSP completion.
    if type(state)is not CutoffState or not _finite(dt) or type(capacity)is not int or capacity<0 or type(capabilities)is not frozenset:
        return Transition('INVALID',state)
    if not (_finite32(state.current) and _finite32(state.target)) or not _u64(state.generation) or not _u64(state.token) or state.token==(1<<64)-1 or type(state.active)is not bool or type(state.awaiting)is not bool:
        return Transition('INVALID',state)
    if state.awaiting:return Transition('AWAITING_ACK',state)
    if not state.active:return Transition('OK_STATE',state)
    if capacity<2:return Transition('CAPACITY',state)
    if not frozenset({'cutoff_apply','global_dispatch'})<=capabilities:return Transition('MISSING_CAPABILITY',state)
    target=F(state.target)
    target=F(0. if target<0. else 1. if target>1. else target)
    try:
        if dt>0:
            if exp_fn is None:return Transition('MISSING_EXP',state)
            exponent=F(exp_fn(fdiv(F(dt),F(-.015))))
            if not _finite(exponent) or not 0.<=exponent<=1.:return Transition('MATH_FAILURE',state)
            alpha=fsub(1.,exponent)
            current=fadd(F(state.current),fmul(fsub(target,F(state.current)),alpha))
        else:current=target
        emitted=F(0. if current<0. else 1. if current>1. else current)
    except (ValueError,OverflowError,TypeError):return Transition('MATH_FAILURE',state)
    stored=target if F(abs(emitted-target))<=EPS else emitted
    nxt=replace(state,current=stored,target=target,token=state.token+1,awaiting=True)
    commands=(Command('cutoff_apply','normalized_control','Core.cutoff',7,emitted,state.generation,0,token=nxt.token),
              Command('global_dispatch','normalized_control','Conductor.global',7,emitted,state.generation,1,token=nxt.token))
    return Transition('PENDING_COMMAND_ACK',nxt,commands)
def cutoff_ack(state,generation,token,actual_cutoff):
    if type(state)is not CutoffState:return Transition('INVALID',state)
    if not all(_finite32(v)for v in (state.current,state.target,actual_cutoff)) or not all(_u64(v)for v in (state.generation,state.token,generation,token)) or type(state.active)is not bool or type(state.awaiting)is not bool:return Transition('INVALID',state)
    if not state.awaiting or generation!=state.generation or token!=state.token:return Transition('STALE_ACK',state)
    def close32(a,b):
        # Finite binary32 endpoints may subtract to infinity in C++; that is
        # a valid non-close result, not a Python struct.pack overflow exception.
        difference=F(a)-F(b)
        return _finite32(difference) and F(abs(F(difference)))<=EPS
    close=close32(state.current,state.target) and close32(actual_cutoff,state.target)
    return Transition('OK_STATE',replace(state,active=not close,awaiting=False))
def free_block_time(frames,sample_rate):
    """Top-level render frames only. Never call separately per internal voice chunk."""
    if type(frames)is not int or not 0<=frames<1<<31 or not _finite(sample_rate) or F(sample_rate)<=0:raise ValueError('invalid render block')
    value=fdiv(frames,F(sample_rate))
    if not math.isfinite(value):raise ValueError('invalid block duration')
    return value
@dataclass(frozen=True)
class BeatState:
    generation: int
    last: float
    accumulator: float=0.
def beat_catchup(state,beat,tempo,loaded,capacity):
    if type(state)is not BeatState or not _u64(state.generation) or not all(_finite(x)for x in (state.last,state.accumulator,beat,tempo)) or not 0<=state.accumulator<F(.0125) or type(loaded)is not bool or type(capacity)is not int or capacity<0:
        return Transition('INVALID',state)
    last,acc,beat,tempo=F(state.last),F(state.accumulator),F(beat),F(tempo)
    step=F(.0125)
    if not loaded or beat<last:return Transition('OK_CLOCK',replace(state,last=beat,accumulator=0.))
    elapsed=fdiv(fmul(fsub(beat,last),F(60.)),F(max(tempo,1.)))
    if not math.isfinite(elapsed) or elapsed<=0:return Transition('OK_CLOCK',replace(state,last=beat,accumulator=acc))
    if elapsed>F(.2):return Transition('OK_CLOCK',replace(state,last=beat,accumulator=0.))
    acc=fadd(acc,elapsed);commands=[]
    # Bound loop before allocating; admitted accumulator is a previous residual.
    while acc>=step:
        if len(commands)==capacity:return Transition('CAPACITY',state)
        commands.append(Command('tick_lfo','elapsed_seconds','Conductor',0,step,state.generation,len(commands)))
        acc=fadd(acc,-step)
    return Transition('OK_CLOCK',replace(state,last=beat,accumulator=acc),tuple(commands))
@dataclass(frozen=True)
class NoteState:
    modes: tuple
    retriggers: tuple=(False,False,False)
    armed: bool=False
    retrigger4: bool=False
    new_note: bool=False
    delay_finished: tuple=(False,False,False)
    delay_passed: tuple=(0.,0.,0.)
def enter_note(state,held_count):
    if type(state)is not NoteState or type(held_count)is not int or held_count<0 or len(state.modes)!=3 or any(type(m)is not int or m not in range(4)for m in state.modes):
        return Transition('INVALID',state)
    if any(len(v)!=3 for v in (state.retriggers,state.delay_finished,state.delay_passed)):return Transition('INVALID',state)
    if any(type(x)is not bool for x in state.retriggers+state.delay_finished) or any(not _finite(x) or x<0 for x in state.delay_passed):return Transition('INVALID',state)
    retriggers=tuple(r or m==2 for r,m in zip(state.retriggers,state.modes))
    changes=dict(retriggers=retriggers,armed=True,retrigger4=True)
    if held_count==0:changes.update(new_note=True,delay_finished=(False,False,False),delay_passed=(0.,0.,0.))
    return Transition('OK_STATE',replace(state,**changes))
