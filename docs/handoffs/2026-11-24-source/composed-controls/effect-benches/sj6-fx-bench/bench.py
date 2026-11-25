#!/usr/bin/env python3
"""Local deterministic SuperJ6 isolated-effect bench. Numeric output only."""
import argparse
import ctypes as C
import hashlib
import json
import math
from pathlib import Path
import sys
import numpy as np

PIN = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
SR = 44100
CASES = ('delay', 'j60-i', 'j60-ii', 'j60-both', 'room-i', 'room-ii', 'widen')

def stimulus(n, tone):
    out = np.zeros((2, n), dtype=np.float32)
    if tone:
        count = min(n, 4096)
        t = np.arange(count)/SR
        env = np.sin(np.pi * np.arange(count)/count)**2
        out[0,:count] = env*(.12*np.sin(2*np.pi*317*t)+.05*np.sin(2*np.pi*1729*t))
        out[1,:count] = env*(.10*np.sin(2*np.pi*487*t)+.04*np.sin(2*np.pi*2851*t))
    else:
        out[0,0] = .5
        if n > 173:
            out[1,173] = -.375
    return out

libm = C.CDLL('libm.so.6')
fma = libm.fmaf
fma.restype = C.c_float
fma.argtypes = [C.c_float]*3

def delay_reference(x, enabled, detent=8, feedback_detent=64):
    # Only the documented settled, no-ping-pong sub-case of the original
    # core, not an execution of original ARM64/audio clock rampers.
    raw = np.float32(detent)/np.float32(127)
    tap = float(np.float32(np.float32(1.5)*raw)*np.float32(SR))
    delay_i = math.floor(tap)
    frac = np.float32(tap-delay_i)
    feedback = np.float32(.99)*(np.float32(feedback_detent)/np.float32(127))
    dry = np.float32(.1 if enabled else 1)
    wet = np.float32(1)-dry
    history = np.zeros(x.size//2+1, np.float32)
    y = np.empty_like(x)
    for ch in range(2):
        history.fill(0)
        for i, value in enumerate(x[ch]):
            j = i-delay_i
            a = history[j] if j >= 0 else 0.0
            b = history[j-1] if j > 0 else 0.0
            tap_value = fma(float(frac), float(b-a), float(a))
            history[i] = fma(float(feedback), tap_value, float(value))
            y[ch,i] = fma(float(wet), tap_value, float(dry*value))
    return y

def audited_delay_reference(x, enabled, tools):
    # Use the existing machine-code-qualified C behavioral port; not a new
    # approximation of delay or a whole-graph AU execution.
    sys.path.insert(0,str(tools))
    from sj6_stereo_delay import delay
    class Params:
        props={'time':.25, 'feedback':.99*64/127,
               'dryWetMix':1.-.9 if enabled else 1., 'pingPong':0.}
    # Call the old reference with the node running in both arms; the AU
    # captures history even at exact dry=1.
    return delay(x,Params(),True,sample_rate=SR).astype('f4')

def revsc_reference(x, enabled, path, tools):
    if not enabled:
        return x.copy()
    sys.path.insert(0,str(tools))
    from emu import Emu
    e=Emu(str(path))
    ABI={'_sp_revsc_create':0x1003a3f08,'_sp_revsc_init':0x1003a3f34,
         '_sp_revsc_compute':0x1003a4180,'_sp_revsc_destroy':0x1003a414c}
    if any(e.im.sym_by_name.get(k)!=v for k,v in ABI.items()):
        raise RuntimeError('pinned RevSC symbol/ABI mismatch')
    sp, indirect, frame = e.calloc(256),e.calloc(8),e.calloc(16)
    e.w32(sp+8,SR);e.w32(sp+12,2) # source layout and ABI load at init+0x2c
    if e.call(ABI['_sp_revsc_create'],indirect)!=1:
        raise RuntimeError('RevSC create refused')
    effect=e.r64(indirect)
    if e.call(ABI['_sp_revsc_init'],sp,effect)!=1:
        raise RuntimeError('RevSC init refused')
    y=np.empty_like(x)
    for i in range(x.shape[1]):
        e.wf(frame,float(x[0,i]));e.wf(frame+4,float(x[1,i]))
        if e.call(ABI['_sp_revsc_compute'],sp,effect,frame,frame+4,frame+8,frame+12)!=1:
            raise RuntimeError(f'RevSC rejected frame {i}')
        y[:,i]=e.rf(frame+8),e.rf(frame+12)
    e.call(ABI['_sp_revsc_destroy'],indirect)
    if e.unknown:
        raise RuntimeError(f'RevSC used unresolved imports: {sorted(e.unknown)}')
    return y

def j60_reference(x, enabled, mode, path, tools):
    sys.path.insert(0,str(tools))
    from emu import Emu
    from j60 import J60
    e = Emu(str(path))
    j = J60(e, mode, 1., 1., 1., sr=SR)
    if not enabled:
        e.wr(j.obj+0x29,b'\x00')  # true started/bypass, not mix=0
    y = j.process(x)
    if e.unknown:
        raise RuntimeError(f'Original kernel used unresolved imported symbols: {sorted(e.unknown)}')
    return y

def compact(shared, name, x, enabled):
    library = C.CDLL(str(shared.resolve()))
    fn = library.sj6_fx_render
    fn.argtypes = [C.c_uint, C.c_uint,
                   np.ctypeslib.ndpointer(dtype=np.float32, ndim=2, flags='C_CONTIGUOUS'),
                   C.c_uint]
    fn.restype = C.c_int
    out = np.ascontiguousarray(x.copy())
    code=fn(CASES.index(name),int(enabled),out,out.shape[1])
    if code != 0:
        raise RuntimeError(f'{name} compact callable refused ({code})')
    return out

def outputs(y, x):
    assert y.shape == x.shape and np.isfinite(y).all()
    tail = y[:, min(y.shape[1],4096):]
    return {'max_abs':float(np.max(np.abs(y))),
            'rms':float(np.sqrt(np.mean(y.astype('f8')**2))),
            'dry_bit_exact':y.tobytes() == x.tobytes(),
            'tail_rms':float(np.sqrt(np.mean(tail.astype('f8')**2))) if tail.size else None}

def self_test():
    x=np.zeros((2,2048),np.float32);x[0,0]=1
    y=delay_reference(x,True,detent=1,feedback_detent=0)
    b=delay_reference(x,False,detent=1,feedback_detent=0)
    tap=np.float32(np.float32(1.5)/np.float32(127))*np.float32(SR)
    i=int(tap)
    assert b.tobytes()==x.tobytes() and not np.any(y[1])
    assert np.argmax(np.abs(y[0])) in (i,i+1) and abs(float(y[0,i:i+2].sum())-.9)<1e-6
    assert stimulus(4096,True).tobytes()==stimulus(4096,True).tobytes()
    return {'passed': True, 'scope':'fixture impulse, original-subcase split tap and channel isolation; no candidate'}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--self-test',action='store_true')
    p.add_argument('--fx',choices=CASES+('revsc-kernel',))
    p.add_argument('--frames',type=int,default=32768)
    p.add_argument('--stimulus',choices=['impulse','tone'],default='impulse')
    p.add_argument('--au',type=Path,help='local private, unpacked authorized Mach-O')
    p.add_argument('--au-bench',type=Path,help='authorized sj6-arm64-bench bench/ directory')
    p.add_argument('--delay-reference',type=Path,
                   default=Path(__file__).resolve().parent.parent/'sj6-multirate',
                   help='directory with prior Build9-verified C/Python StereoDelay port')
    p.add_argument('--compact-so',type=Path,help='separately approved compiled candidate bridge')
    args=p.parse_args()
    if args.self_test:
        print(json.dumps(self_test(),indent=2));return
    if not args.fx or not 256<=args.frames<=SR*5:
        p.error('expected effect and 256 to 220500 frames')
    x=stimulus(args.frames,args.stimulus=='tone')
    raw_hash=hashlib.sha256(x.tobytes()).hexdigest()
    row={'effect':args.fx,'rate':SR,'frames':args.frames,'stimulus':args.stimulus,
         'stimulus_sha256':raw_hash}
    ref=cand=None
    if args.fx=='delay':
        if (args.delay_reference/'sj6_stereo_delay.c').is_file():
            row['reference']='prior Build9-verified original-equation C stereo delay kernel; settled 250 ms/120 BPM, no live ramp or original graph'
            ref=[audited_delay_reference(x,v,args.delay_reference) for v in (False,True)]
        else:
            row['reference']='unavailable; point --delay-reference to the existing checked C port'
    if (args.fx.startswith('j60') or args.fx=='revsc-kernel') and args.au and args.au_bench:
        if hashlib.sha256(args.au.read_bytes()).hexdigest()!=PIN:
            p.error('unexpected original AU; refusal to use unpinned entry addresses')
        if args.fx=='revsc-kernel':
            row['reference']='actual raw ARM64 Build9 Costello RevSC defaults; no Epic wrapper/Type/HPF/limiter/mix; no compact equivalent asserted'
            ref=[revsc_reference(x,v,args.au,args.au_bench) for v in (False,True)]
        else:
            row['reference']='actual ARM64 isolated Build9 J60V2 kernel with matched controls, not full original graph'
            ref=[j60_reference(x,v,CASES.index(args.fx),args.au,args.au_bench) for v in (False,True)]
    row.setdefault('reference','unavailable; no substitute used')
    if args.compact_so:
        if args.fx=='revsc-kernel':
            p.error('raw RevSC is not the same graph as compact Room I/II; no fabricated pairing')
        cand=[compact(args.compact_so,args.fx,x,v) for v in (False,True)]
        row['compact']=[outputs(y,x) for y in cand]
    if ref:
        row['original']=[outputs(y,x) for y in ref]
    if ref and cand:
        # Compare WET - DRY from each engine without gain or time normalization.
        error = (cand[1].astype('f8')-cand[0])-(ref[1].astype('f8')-ref[0])
        row['null']={'rms':float(np.sqrt(np.mean(error**2))),'peak':float(np.max(np.abs(error)))}
    print(json.dumps(row,indent=2,allow_nan=False))

if __name__=='__main__': main()
