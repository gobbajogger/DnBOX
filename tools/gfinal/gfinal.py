#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""G-FINAL: shared type-based glyph reference, ported from DnBOX 0.0.26.

python3 gfinal.py --all        writes PNGs, sheets, GIFs, hashes and self-test
python3 gfinal.py --glyph CUTOFF --value 64 --out cutoff.png

26x19 is the canonical engine-independent raster. 40x24 is that SAME raster
centred at (7,2). 4x always means nearest neighbour, not more detailed artwork.
Live input is engine state, never a simulated measurement in production.
Demo animation calls demo_state(), explicitly a synthetic test fixture.
"""
import math, json, hashlib, argparse
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
ROOT=Path(__file__).resolve().parent
W,H,SS=26,19,4
# RGB565-quantized colours are normative; opaque RGB PNG is the reference format.
def rgb565(c):
    r,g,b=c; return ((r>>3)*255//31,(g>>2)*255//63,(b>>3)*255//31)
BG=rgb565((16,26,31)); INK=rgb565((255,187,65)); TRACK=rgb565((50,70,75)); WHITE=rgb565((238,245,230)); GRAY=rgb565((135,155,150))
# id | one-line meaning | release ancestor | required fix vs release.
SPEC='''LEVEL|Loudness / output energy|orig_strip_glyph level, DG_LEVEL|One common meter; actual engine VCA fills it
PAN|Position between left and right|control_glyph pan|Common centred bipolar scale
STEREO|Distance between left and right voices|orig_strip_glyph stereo|Common spread, no engine-specific variant
WAVE|Morph from saw to pulse|draw_wave_glyph / original_wave|Use real shape, not generic fallback bars
WAVE_SHAPE|Selected oscillator or LFO waveform|draw_wave_glyph|Distinct exact sine / triangle / saw / pulse states
SUB|Loudness of the lower octave|orig_strip_glyph sub|Half-frequency sub, shared by every synth
NOISE|Loudness of noise|DG_SNAPPY noise component|Separate broadband-noise waveform from envelope tail
TUNE|Oscillation pitch, low to high|DG_TUNE and orig_strip_glyph tune|Use one wave+centre mark, no misleading kick sweep
GLIDE|Time to slide between notes|orig_strip_glyph glide|Common time-width ramp and live pitch cursor
PWM|Pulse duty cycle|original_wave pulse|Keep vertical edges and explicit pulse width
FM|Depth of frequency modulation|og_fm|Same phase-modulation schematic for every engine
DRIVE|Soft clipping / saturation|og_bite, drive_amount_glyph|Common normalized sine-to-flat-top law
FOLD|Amount of wave folding|og_gnaw|Common reflected sine
BITS|Resolution of amplitude steps|og_crush|Separate bit depth from sample rate
SAMPLE_RATE|How often the waveform is sampled|og_crush|Separate held samples from amplitude quantization
DISTORTION|Selected clipping or folding transfer|drive_amount_glyph / orig_fx_glyph|Distinct curves replace anonymous switch pips
CUTOFF|Low-pass corner frequency|orig_strip_glyph cutoff|Keep familiar LP contour; common geometry and live applied cutoff
HIGHPASS|Low frequencies removed below corner|filter_glyph mode 3|Correct old low-cut drawn as low-pass
RESONANCE|Emphasis at the filter corner|orig_strip_glyph peak|Peak height changes, fixed corner unless engine supplies it
FILTER_MODE|Selected off / LP / BP / HP filter|filter_glyph|All mode states visible, not a flat macro
BANDWIDTH|Width of the frequencies passed|filter_glyph band-pass|Move both edges, not resonance height
ENV_AMOUNT|Signed depth of filter envelope sweep|orig_strip_glyph ENV|Show target knee even at rest; retain live EG knee
ATTACK|Time to rise to full envelope level|draw_adsr_inline|Common ADSR with edited attack highlighted
DECAY|Time for the initial sound to fade|DG_DECAY and draw_adsr_inline|Common exponential tail and actual live progress
SUSTAIN|Envelope level while a note is held|draw_adsr_inline|Common ADSR with sustain segment highlighted
RELEASE|Time to fade after note-off|draw_adsr_inline|Common ADSR with release segment highlighted
GATE_MODE|Envelope shape or on/off gate|orig_strip_glyph amp mode|Distinct gate and ADSR silhouettes
TRANSIENT|Strength of the initial click / hit|DG_ATTACK|Click-height, not mislabeled attack-time ramp
PITCH_DECAY|Time for a kick pitch to fall|DG_BDSWEEP|Highlight time, separate from pitch and depth
PITCH_DEPTH|Distance of the kick pitch drop|DG_BDSWEEP|Highlight drop depth
PITCH_BASE|Starting / base pitch of the kick|DG_BDSWEEP|Make pitch actually move (old snapshot could stay static)
SNAPPY|Loudness of the snare noise tail|DG_SNAPPY|Common noise-tail drawing, real hit envelope
LFO_RATE|Speed of the modulation wave|og_lfo_rate|Same scroll + phase dot for all engines
LFO_DEPTH|Size of the modulation wave|og_lfo_amp|Same depth + phase dot for all engines
LFO_DELAY|Wait before the modulation fades in|NEW (SuperJ6 lfoNDelayTime)|Flat wait then a sine growing in; length follows value
CHORUS|Depth of detuned chorus voices|orig_fx_glyph chorus / ensemble|Voices separate with amount, actual modulator phase
PHASER|Depth or width of moving filter notches|orig_fx_glyph phaser|Notches move with supplied phase
RINGMOD|Depth of amplitude ring modulation|orig_fx_glyph ringmod|Real carrier times modulator schematic
DELAY_TIME|Spacing between echoed hits|all_glyph_semantics delay time|Common tap spacing, no frozen 1/8 drawing
FEEDBACK|How strongly each echo repeats|semantic_amount_glyph feedback|Common successive tap attenuation
DELAY_SEND|Level of the delayed sound|semantic_amount_glyph delay send|All tap amplitudes follow send value
REVERB_SEND|Level of diffuse reverberation|semantic_amount_glyph reverb send|Diffuse tail amplitudes follow send value
TONE|Tilt from dark to bright sound|all_glyph_semantics tone|Bipolar shelf, not wrong LP for low cut
TAPE|Tape reels, motion and wear|all_glyph_semantics tape|True circular reels, two-row tape path and rotating spokes
WEAR|Dropouts / damage in tape|all_glyph_semantics tape wear|Visible gaps in a tape strip instead of generic bars
DELAY_TYPE|Digital taps or tape delay|all_glyph_semantics tape and delay|Real digital / tape identities
PINGPONG|Echo stays centred or alternates L/R|semantic_amount_glyph feedback|Distinct upper/lower channel taps
THRESHOLD|Level above which compression starts|comp_glyph_output|Threshold line on common transfer curve
RATIO|Compression slope above the knee|comp_glyph_output|Real 1:ratio slope, not generic bars
MAKEUP|Output gain after compression|semantic_amount_glyph compressor|Lift the transfer curve above unity
BLEND|Crossfade between dry and wet signals|orig_strip_glyph stereo / DG_SHELLS|Two source bars crossfade, never a volume meter
COMPRESSOR|Combined compression transfer curve|comp_glyph_output|Soft knee, threshold and live input/output point
LIMITER|Ceiling that stops output peaks|comp_glyph_output|Flat ceiling, visible bypass state
PUMP|Sidechain dip and recovery|perform_glyph pump|Restore the original dip; trigger from actual source
SOURCE|Which source drives this control (pump trigger, LFO slot)|glyph_identity, mirrored DESTINATION|One routing picture instead of static pips
TEMPO|Beat speed / metronome|perform_glyph tempo|Tempo value changes arm range; clock phase drives live arm
SWING|How late the offbeat occurs|perform_glyph swing|Same shifted middle tick
LENGTH|How many sequence steps play|all_glyph_semantics length|Common eight-cell summary; full value remains labeled
REPEAT|Number of hits repeated in a step|semantic_amount_glyph repeat|Same growing train of pulses
RATE|Spacing of events / clock division|all_glyph_semantics rate|Same pulse train; adapter supplies actual division
DIRECTION|Forward, backward, both or random|control_glyph direction|Four distinct arrow trajectories
SLOT|Which stored pattern / preset / bar / engine is selected|glyph_identity|One shared selected-cell grid (was generic bars for PRESET)
NOTE|Pitch of a sequencer note|piano-roll note|Common two-key ledger and vertically moving note
GATE|How long a sequencer note is held|orig_strip_glyph gate|Pulse width along the time axis
CHANCE|Probability that a step plays|glyph_identity + repeat|Eight outcomes progressively enabled, no static dice
SLIDE|Pitch transition connecting two steps|orig_strip_glyph glide|Two notes connected by a value-driven pitch slide
LINK|Whether two controls are linked|NEW (SuperJ6 link enables)|Two true circular chain links; open / connected state
OCTAVE|Number of octaves / octave offset|orig_strip_glyph range|Common ascending octave stairs
KEY_RANGE|Low or high note boundary|piano-roll note|A keyboard interval with a moving boundary
TRACKING|How strongly key pitch affects a control|orig_strip_glyph pitch and filter|Moving mapping slope, not arbitrary bars
PEDAL|Sustain pedal: how far notes ring on|matrix_glyph GL_PEDAL|2-state (0.0.26 Damper is ON at >=64): pedal up/down + note ring-out tail
ENABLE|Bypass / enabled switch|matrix_glyph switch|Open / closed circuit; no decorative idle motion
DESTINATION|Selected modulation destination|NEW (SuperJ6/Synth One routing)|Route line to one highlighted destination node
SCALE|Set of pitches allowed in the scale|piano-roll note|Keyboard pitch-class mask; adapter supplies actual mask
ROOT|Pitch class on which a scale starts|piano-roll note|Highlight selected pitch class on a two-row keyboard
UNISON|Several voices sounding together|orig_fx_glyph ensemble|Grow number of parallel voices
MOD_BASE|Base value before modulation is applied|og_lfo_amp / bipolar scale|Horizontal value marker plus live modulation offset
MOD_MODE|Free-running, retriggered or single modulation|og_lfo_rate|Three distinct cycle / restart / one-shot paths
LEGATO|Notes joined without retriggering|orig_strip_glyph glide / gate|Separated or joined notes, no arbitrary animation
LOOP|Whether the sequence repeats|orig_fx_glyph bypass / direction|Open path or closed return loop
DENSITY|How many steps contain a hit|semantic_amount_glyph repeat|Common populated step grid instead of volume fallback
MUTATE|How much the sequence is varied|glyph_identity|Two step rows, progressively changed notes
'''
GLYPHS={}
for line in SPEC.strip().splitlines():
    gid,meaning,ancestor,fix=line.split('|'); GLYPHS[gid]={'id':gid,'meaning':meaning,'ancestor':ancestor,'fix':fix,'new':ancestor=='NEW'}
# Provenance vs the 0.0.26 (== 0.0.25.1) glyph code.
# REUSED: 0.0.26 drawing law kept; only deterministic AA + shared 26x19 cell.
# FIXED:  0.0.26 glyph (or the param's 0.0.26 fallback) corrected / redrawn to meet the requirements.
# NEW:    no 0.0.26 glyph or parameter existed (SuperJ6 / Synth One era types).
NEW_IDS=set('LFO_DELAY LINK DESTINATION KEY_RANGE NOTE MOD_BASE UNISON MOD_MODE LEGATO TRACKING'.split())
REUSED_IDS=set('''LEVEL PAN STEREO FM DRIVE CUTOFF RESONANCE DECAY ATTACK SUSTAIN RELEASE SNAPPY LFO_DEPTH CHORUS PHASER
FEEDBACK DELAY_SEND DELAY_TIME SWING REPEAT RATE DIRECTION SLOT GATE PUMP COMPRESSOR THRESHOLD RATIO GATE_MODE ENABLE
PITCH_DECAY PITCH_DEPTH'''.split())
for g in GLYPHS.values():
    g['class']='NEW' if g['id'] in NEW_IDS else 'REUSED' if g['id'] in REUSED_IDS else 'FIXED'
    g['new']=g['class']=='NEW'
# All glyphs are value/state driven. Live paths are additional to value geometry.
LIVE=set('LFO_DELAY LEVEL STEREO WAVE SUB NOISE TUNE GLIDE PWM FM DRIVE FOLD BITS SAMPLE_RATE CUTOFF HIGHPASS RESONANCE ENV_AMOUNT ATTACK DECAY SUSTAIN RELEASE ENVELOPE TRANSIENT PITCH_DECAY PITCH_DEPTH PITCH_BASE SNAPPY LFO_RATE LFO_DEPTH CHORUS PHASER RINGMOD DELAY_TIME FEEDBACK DELAY_SEND REVERB_SEND TAPE COMPRESSOR PUMP TEMPO NOTE GATE MOTION TRACKING PEDAL MOD_BASE'.split())
# Only the following independent actual engine quantities affect the live paths.
# Every adapter must supply these from DSP state; omission means idle, not fake motion.
# amp, env, filter_env, cutoff, resonance in 0..1; phase in cycles; progress in 0..1.
# tape_phase in cycles; tape_active bool; clock_phase 0..1 and playing bool.
STATES={'PEDAL':2,'OCTAVE':4,'DESTINATION':3,'WAVE_SHAPE':4,'DISTORTION':8,'FILTER_MODE':4,'GATE_MODE':2,'DELAY_TYPE':2,'PINGPONG':2,'LIMITER':2,'SOURCE':3,'DIRECTION':4,'LINK':2,'ENABLE':2,'MOD_MODE':3,'LEGATO':2,'LOOP':2}
VALUES=[0,32,64,96,127]

def clamp(x,a=0,b=1):return max(a,min(b,x))
def rnd(x):return int(math.floor(x+0.5))
# ---- Portable math. Firmware port: IEEE double, -ffp-contract=off, same operation
# order, and THESE tables (emit via --emit-c); never libm sin/exp/tanh.
SIN_T=[rnd(math.sin(2*math.pi*i/1024)*16384) for i in range(1025)]
EXP_T=[rnd(math.exp(-i/64)*1048576) for i in range(1025)]
TWO_PI=6.283185307179586
def gsin_c(t):
    t=t-math.floor(t);x=t*1024.0;i=int(math.floor(x));f=x-i
    return (SIN_T[i]+(SIN_T[i+1]-SIN_T[i])*f)/16384.0
def gsin_r(a):return gsin_c(a/TWO_PI)
def gcos_r(a):return gsin_c(a/TWO_PI+0.25)
def gexp(x):
    # e^x for x<=0 (clamped at -16); x>0 only used through gtanh symmetry.
    if x>0:return 1.0/gexp(-x)
    y=-x*64.0
    if y>=1024.0:return EXP_T[1024]/1048576.0
    i=int(math.floor(y));f=y-i
    return (EXP_T[i]+(EXP_T[i+1]-EXP_T[i])*f)/1048576.0
def gtanh(x):
    e=gexp(-2.0*abs(x));v=(1.0-e)/(1.0+e);return v if x>=0 else -v
ROLES=('track','gray','ink','white');ROLE_OF={TRACK:'track',GRAY:'gray',INK:'ink',WHITE:'white'}
SX=None
class Canvas:
    """Deterministic 4x4 supersampled coverage rasterizer (integer Q4 geometry).
    Pixel centres are integer coordinates. A sample is inside a 1 px stroke when
    its distance to the segment is <= 0.5 px. Coverage = inside samples / 16."""
    def __init__(self,gid=''):
        import numpy as np
        self.np=np;self.gid=gid
        self.m={r:np.zeros((H*SS,W*SS),bool) for r in ROLES}
        gx=np.arange(W*SS);gy=np.arange(H*SS)
        self.sx=(gx//SS)*16-6+(gx%SS)*4;self.sy=(gy//SS)*16-6+(gy%SS)*4
        self.bounds=[]
    @staticmethod
    def q(v):return rnd(v*16)
    def _seg(self,role,a,b,hw):
        np=self.np;ax,ay=self.q(a[0]),self.q(a[1]);bx,by=self.q(b[0]),self.q(b[1])
        x0=min(ax,bx)-hw;x1=max(ax,bx)+hw;y0=min(ay,by)-hw;y1=max(ay,by)+hw
        cx=np.nonzero((self.sx>=x0)&(self.sx<=x1))[0];cy=np.nonzero((self.sy>=y0)&(self.sy<=y1))[0]
        if not len(cx) or not len(cy):return
        X=self.sx[cx][None,:].astype(np.int64);Y=self.sy[cy][:,None].astype(np.int64)
        dx=bx-ax;dy=by-ay;l2=dx*dx+dy*dy;px=X-ax;py=Y-ay;h2=hw*hw
        if l2==0:ins=px*px+py*py<=h2
        else:
            t=px*dx+py*dy;cr=px*dy-py*dx
            ins=np.where(t<=0,px*px+py*py<=h2,np.where(t>=l2,(X-bx)**2+(Y-by)**2<=h2,cr*cr<=h2*l2))
        self.m[role][cy[0]:cy[-1]+1,cx[0]:cx[-1]+1]|=ins
    def line(self,p,col=INK,width=1):
        self.bounds+=list(p);role=ROLE_OF[col]
        for a,b in zip(p,p[1:]):self._seg(role,a,b,8*width)
        if len(p)==1:self._seg(role,p[0],p[0],8*width)
    def rect(self,x,y,w,h,col=INK):
        x0=rnd(x);y0=rnd(y);x1=rnd(x+w);y1=rnd(y+h)
        if x1<=x0 or y1<=y0:return
        self.bounds.extend([(x0,y0),(x1-1,y1-1)])
        self.m[ROLE_OF[col]][y0*SS:y1*SS,x0*SS:x1*SS]=True
    def erase(self,x,y,w,h):
        for r in ROLES:self.m[r][y*SS:(y+h)*SS,x*SS:(x+w)*SS]=False
    def circle(self,x,y,r,col=INK,fill=False):
        np=self.np;self.bounds.extend([(x-r,y-r),(x+r,y+r)])
        cxq,cyq,rq=self.q(x),self.q(y),self.q(r)
        X=self.sx[None,:].astype(np.int64)-cxq;Y=self.sy[:,None].astype(np.int64)-cyq;d2=X*X+Y*Y
        ins=d2<=(rq+8)**2 if fill else (d2<=(rq+8)**2)&(d2>=max(0,rq-8)**2)
        self.m[ROLE_OF[col]]|=ins
    def wave(self,fn,amp=6,mid=9,col=INK,x0=1,x1=24,n=92):
        self.line([(x,mid-amp*clamp(fn((x-x0)/(x1-x0)),-1,1)) for x in [x0+i*(x1-x0)/n for i in range(n+1)]],col)
    def arrow(self,x0,y0,x1,y1,col=INK):
        self.line([(x0,y0),(x1,y1)],col);dx=x1-x0;dy=y1-y0;l=math.sqrt(dx*dx+dy*dy);ux=dx/l;uy=dy/l
        C,S=0.8253356149096783,0.5646424733950354
        self.line([(x1-3*(ux*C-uy*S),y1-3*(uy*C+ux*S)),(x1,y1),(x1-3*(ux*C+uy*S),y1-3*(uy*C-ux*S))],col)
    def layers(self):
        np=self.np
        return {r:self.m[r].reshape(H,SS,W,SS).sum(axis=(1,3)).astype(np.uint8) for r in ROLES}
    def finish(self):
        for x,y in self.bounds:
            if not (0.5<=x<=24.5 and 0.5<=y<=17.5):raise AssertionError(('out of bounds',self.gid,x,y))
        L=self.layers();np=self.np
        out=np.empty((H,W,3),np.int64);out[:]=BG
        for r,col in zip(ROLES,(TRACK,GRAY,INK,WHITE)):
            a=L[r].astype(np.int64)[:,:,None];out=(out*(16-a)+np.array(col)*a+8)>>4
        img=Image.fromarray(out.astype(np.uint8),'RGB')
        img.putdata([rgb565(c) for c in img.get_flattened_data()])
        img.info['layers']=b''.join(L[r].tobytes() for r in ROLES);return img

def render(gid,value=64,live=None,*,state=None,cell='compact',scale=1,context=None):
    """Pure deterministic renderer; q is 0..127; selector state wins over q.
    Context may supply actual enumerated selector mask, envelope ADSR, cutoff,
    resonance, and (for WAVE) true oscillator shape/morph. No engine identifier
    is accepted: equal type/value/context/state yields identical glyph pixels.
    """
    if gid not in GLYPHS:raise KeyError(gid)
    q=clamp(value/127);live=live or {};ctx=context or {};c=Canvas(gid)
    amp=clamp(live.get('amp',0));env=clamp(live.get('env',0));ph=live.get('phase',0)%1
    active=bool(live.get('active',False));p=clamp(live.get('progress',0));k=1+.0
    n=STATES.get(gid);s=max(0,min(n-1,state if state is not None else rnd(q*(n-1)))) if n else 0
    # Ink schematic never pretends to be a measured spectrum. White is actual live state only.
    def baseline():c.line([(1,17),(24,17)],TRACK)
    def dot(x,y):c.rect(clamp(x,2,23)-1,clamp(y,2,16)-1,3,3,WHITE)
    def signal_pips():
        if active:
            for i in range(rnd(amp*3)):c.rect(2+i*4,16,2,1,WHITE)
    def sine(t,f=1.5):return gsin_c(t*f)
    def filter_curve(mode='LP',cut=None,res=None,ink=INK):
        cq=q if cut is None else clamp(cut);rq=.12 if res is None else clamp(res)
        # 26px release contour, now safe at both extremes.
        x=5+14*cq;top=7;peak=top-5*rq
        pts=([(1,top),(x-2,top),(x,peak),(x+4,16),(24,16)] if mode=='LP' else
             [(1,16),(x-4,16),(x,peak),(x+2,top),(24,top)] if mode=='HP' else
             [(1,16),(x-4,16),(x,peak),(x+4,16),(24,16)])
        c.line(pts,ink)
    def envshape(which):
        a,d,su,r=ctx.get('adsr',(.2,.3,.6,.3))
        if which=='ATTACK':a=.02+.88*q
        elif which=='SUSTAIN':su=q
        elif which=='RELEASE':r=.02+.88*q
        elif which=='ENVELOPE':a=.06+.35*q;d=.12+.4*q;su=.2+.6*q;r=.06+.5*q
        x1=2+9*a;x2=min(x1+3+4*d,15);x3=17;x4=18+6*r;sy=rnd(16-13*su)
        pts=[(1,16),(x1,3),(x2,sy),(x3,sy),(x4,16),(24,16)]
        for i in range(5):c.line(pts[i:i+2],INK if which=='ENVELOPE' or i=={'ATTACK':0,'SUSTAIN':2,'RELEASE':3}.get(which,-1) else TRACK)
        if active:
            x=1+23*p
            for (xa,ya),(xb,yb) in zip(pts,pts[1:]):
                if xa<=x<=xb: dot(x,ya+(yb-ya)*(x-xa)/max(.001,xb-xa));break
    def tapset(kind='feedback'):
        baseline();feedback=.08+.9*q
        for j in range(4):
            h=(14*(feedback**j) if kind=='feedback' else (14-3*j)*q)
            if h>=.8:c.rect(2+j*7,16-h,2,h,INK)
            if active and abs(p-j/4)<.13:c.rect(2+j*7,15,2,2,WHITE)
    if gid=='LEVEL':
        baseline()
        for j in range(7):
            h=2+j*2;c.rect(3+j*3,16-h,2,h,INK if q>0 and j/6<=q else TRACK)
        if active:
            for j in range(7):
                if j/6<=q*amp:c.rect(3+j*3,15,2,1,WHITE)
    elif gid=='PAN':
        baseline();c.line([(2,9),(23,9)],TRACK);c.rect(12,5,2,9,GRAY)
        c.line([(2,6),(2,12)],GRAY);c.line([(23,6),(23,12)],GRAY)
        c.rect(rnd(3+18*q),6,2,7)
    elif gid=='MOD_BASE':
        # Unipolar base marker; grey bracket = modulation range; white = live modulated value.
        baseline();c.line([(2,9),(23,9)],TRACK);x=rnd(3+18*q);dep=rnd(6*ctx.get('depth',.5))
        lo=max(2,x+1-dep);hi=min(23,x+1+dep);c.line([(lo,12),(hi,12)],GRAY);c.line([(lo,11),(lo,13)],GRAY);c.line([(hi,11),(hi,13)],GRAY)
        c.rect(x,5,2,7)
        if active:dot(clamp(x+1+live.get('mod',0)*dep,2,23),4)
    elif gid=='STEREO':
        c.rect(12,2,2,15,TRACK);sp=rnd(9*q)
        c.rect(10-sp,5,2,9);c.rect(14+sp,5,2,9)
        if active:
            h=rnd(amp*3)
            if h:c.rect(10-sp,14-h,2,h,WHITE);c.rect(14+sp,14-h,2,h,WHITE)
    elif gid in ('WAVE','WAVE_SHAPE','PWM'):
        baseline();c.line([(2,9),(22,9)],TRACK)
        if gid=='PWM':
            w=rnd(10-8*q)
            c.line([(2,15),(2,3),(2+w,3),(2+w,15),(22,15),(22,3)])
        elif gid=='WAVE_SHAPE':
            if s==0:c.wave(lambda t:sine(t,2),amp=6,x0=2,x1=22)
            elif s==1:c.line([(2,9),(4.5,3),(9.5,15),(14.5,3),(19.5,15),(22,9)])
            elif s==2:c.line([(2,15),(12,3),(12,15),(22,3),(22,15)])
            else:c.line([(2,15),(2,3),(7,3),(7,15),(12,15),(12,3),(17,3),(17,15),(22,15),(22,3)])
        else:
            # 0.0.26 draw_wave_glyph law: (1-m)*saw + m*square, two cycles, real vertical edges.
            for x in (2,12):
                c.line([(x,15-12*q),(x+5,9-6*q),(x+5,9+6*q),(x+10,3+12*q),(x+10,15-12*q)])
        signal_pips()
    elif gid=='SUB':
        # 0.0.26 Miami SUB: dim oscillator square above, one-octave-down square below.
        c.line([(2,4),(2,1),(7,1),(7,4),(12,4),(12,1),(17,1),(17,4),(22,4),(22,1)],TRACK)
        a=1+rnd(4*q);y=11
        c.line([(2,y+a),(2,y-a),(12,y-a),(12,y+a),(22,y+a),(22,y-a)])
        baseline();signal_pips()
    elif gid in ('NOISE','SNAPPY'):
        baseline();a=6*q*(.25+.75*amp if active else 1);seed=int(live.get('progress',0)*25) if active else 0
        vals=[(((j+seed)*73+19)%127)/63.5-1 for j in range(24)]
        pts=[]
        for j in range(24):
            k=gexp(-j/9.0) if gid=='SNAPPY' else 1.0
            pts.append((1+j,9-a*k*vals[j]))
        c.line(pts)
        if gid=='SNAPPY':c.line([(1,3),(1,15)],TRACK)
    elif gid=='TUNE':
        baseline();c.line([(12,14),(12,17)],GRAY);c.rect(1+rnd(22*q),15,2,2)
        cyc=1+2.5*q;am=(2+4*amp) if active else 5
        c.wave(lambda t:sine(t,cyc),amp=am,mid=8)
        if active:dot(1+23*p,8-am*sine(p,cyc))
    elif gid in ('GLIDE','SLIDE'):
        baseline();width=rnd(18*q);x0=rnd((25-width)/2)
        pts=[(1,15),(x0,15),(x0+width,3),(24,3)]
        if gid=='SLIDE':
            pts=[(4,15),(x0,15),(x0+width,3),(21,3)];c.rect(1,14,3,3);c.rect(22,2,3,3)
        if active and p<1:
            fx=x0+p*width;fy=15-12*p
            c.line(pts[:2]+[(fx,fy)]);c.line([(fx,fy)]+pts[2:],TRACK);dot(fx,fy)
        else:c.line(pts)
    elif gid in ('FM','DRIVE','FOLD','BITS','SAMPLE_RATE','RINGMOD'):
        baseline();c.line([(1,9),(24,9)],TRACK)
        if gid=='FM':fn=lambda t:gsin_c(1.5*t+q*0.25464790894703254*gsin_c(1.5*t))
        elif gid=='DRIVE':
            g=.3+6*q;fn=lambda t:gtanh(g*sine(t))/gtanh(g)
        elif gid=='FOLD':
            def fn(t):
                v=sine(t)*(1+1.6*q)
                while v>1 or v< -1:v=2-v if v>1 else -2-v
                return v
        elif gid=='RINGMOD':fn=lambda t:gsin_c(3*t)*((1-q)+q*gsin_c(t))
        if gid=='BITS':
            levels=[7,5,4,3,2][min(4,int(q*4.999))] if True else 0
            levels=rnd(6-4*q)
            pts=[];prev=None
            for x in range(1,25):
                v=sine((x-1)/23.0);y=9-rnd(v*levels)*6/levels
                if prev is not None and y!=prev:pts.append((x-.5,prev));pts.append((x-.5,y))
                pts.append((x,y));prev=y
            c.line(pts)
        elif gid=='SAMPLE_RATE':
            steps=[23,12,8,6,4][min(4,rnd(q*4))] if False else max(3,rnd(23-19*q))
            pts=[];xs=[1+rnd(23*j/steps) for j in range(steps+1)]
            for j in range(steps):
                y=9-6*sine(j/steps);pts+=[(xs[j],y),(xs[j+1],y)]
            c.line(pts)
        else:c.wave(fn)
        signal_pips()
    elif gid=='DISTORTION':
        baseline();c.line([(3,15),(22,3)],TRACK)
        funcs=[lambda x:x, lambda x:gtanh(x*2)/gtanh(2),lambda x:clamp(x*3,-1,1),lambda x:abs(x)*2-1,lambda x:gsin_c(x*0.8),lambda x:rnd(x*2)/2,lambda x:abs(x),lambda x:gtanh(x*4+.5)*.75]
        f=funcs[s];c.line([(3+19*i/64,9-6*clamp(f(i/32-1),-1,1)) for i in range(65)])
    elif gid in ('CUTOFF','HIGHPASS','RESONANCE','ENV_AMOUNT','FILTER_MODE','BANDWIDTH'):
        baseline()
        cut=clamp(live.get('cutoff',q if gid in ('CUTOFF','HIGHPASS') else ctx.get('cutoff',.5)))
        res=clamp(live.get('resonance',q if gid=='RESONANCE' else ctx.get('resonance',.12)))
        if gid=='ENV_AMOUNT':
            dep=2*q-1;base=ctx.get('cutoff',.5);bx=5+14*base;target=clamp(base+dep*.45);tx=5+14*target
            c.line([(rnd(bx),15),(rnd(bx),17)],GRAY)
            if abs(tx-bx)>=2:c.arrow(bx,3,tx,3,GRAY)
            cut=clamp(base+dep*.45*clamp(live.get('filter_env',env if active else 0)))
            filter_curve('LP',cut,res)
        elif gid=='FILTER_MODE':
            if s==0:c.line([(1,7),(24,7)],TRACK);c.line([(9,3),(16,12)],INK)
            else:filter_curve(['','LP','BP','HP'][s],.5,.35)
        elif gid=='BANDWIDTH':
            w=rnd(2+6*q);c.line([(1,16),(12-w-3,16),(12-w,5),(13+w,5),(13+w+3,16),(24,16)])
        else:
            filter_curve('HP' if gid=='HIGHPASS' else 'LP',cut,res)
            if active and amp>.05:
                for x in range(2,24,3):
                    passed=(x<5+14*cut)==(gid!='HIGHPASS');h=max(1,rnd((8 if passed else 2)*amp))
                    c.rect(x,16-h,1,h,WHITE)
    elif gid in ('ATTACK','SUSTAIN','RELEASE','ENVELOPE'):
        baseline();envshape(gid)
    elif gid=='DECAY':
        baseline();tau=.035+.38*q;c.line([(1,16),(1,3)],TRACK)
        f=lambda t:16-13*gexp(-t/tau)
        if active:
            n=rnd(92*p);pts=[(1+i*23/92,f(i/92)) for i in range(93)]
            c.line(pts[:n+1],TRACK);c.line(pts[n:]);dot(1+23*p,f(p))
        else:c.line([(1+i*23/92,f(i/92)) for i in range(93)])
    elif gid=='GATE_MODE':
        baseline()
        if s:c.line([(1,16),(4,16),(4,3),(20,3),(20,16),(24,16)])
        else:c.line([(1,16),(5,3),(11,9),(18,9),(24,16)])
    elif gid=='TRANSIENT':
        baseline();h=rnd(2+12*q);c.rect(2,16-h,2,h,WHITE if active and p<.15 else INK)
        body=(amp if active else 1.0)
        c.line([(5,16-6*body),(9,16-4*body),(15,16-2*body),(24,16)])
    elif gid in ('PITCH_DECAY','PITCH_DEPTH','PITCH_BASE'):
        baseline();tau=.08+.45*q if gid=='PITCH_DECAY' else .25
        depth=.05+.75*q if gid=='PITCH_DEPTH' else .6;base=.08+.45*q if gid=='PITCH_BASE' else .15
        ys=lambda t:16-13*clamp(base+depth*gexp(-t/tau))
        c.line([(1+i*23/92,ys(i/92)) for i in range(93)])
        if gid=='PITCH_DEPTH':c.line([(23,rnd(ys(0))),(23,rnd(ys(1)))],GRAY)
        elif gid=='PITCH_DECAY':c.line([(1,17),(rnd(1+23*tau*1.6),17)],GRAY)
        else:c.line([(19,rnd(ys(1))+2),(24,rnd(ys(1))+2)],GRAY)
        if active:dot(1+23*p,ys(p))
    elif gid=='VOICE':
        baseline()
        if s==0:c.line([(1,16),(3,3),(6,10),(12,14),(24,16)])
        elif s==1:c.wave(lambda t:sine(t,6)*gexp(-3*t),amp=7)
        elif s==2:c.line([(1,15),(6,3),(11,15),(16,3),(21,15),(24,9)])
        else:c.wave(lambda t:sine(t,3.5),amp=3)
    elif gid in ('LFO_RATE','LFO_DEPTH'):
        baseline();c.line([(1,9),(24,9)],TRACK);cyc=1+2.5*q if gid=='LFO_RATE' else 2;am=6 if gid=='LFO_RATE' else 1+5*q
        c.wave(lambda t:gsin_c(cyc*t+ph),amp=am)
        if active:dot(24,9-am*gsin_c(cyc+ph))
    elif gid=='LFO_DELAY':
        baseline();c.line([(1,9),(24,9)],TRACK);d=1+rnd(13*q);fade=6
        def f(t):
            x=1+23*t
            if x<=d:return 0.0
            k=min(1.0,(x-d)/fade);return k*gsin_c((x-d)/8.0)
        c.wave(f,amp=6);c.line([(d,6),(d,12)],GRAY)
        if active:dot(1+23*p,9-6*f(p))
    elif gid=='CHORUS':
        baseline();sep=rnd(2+4*q);wob=(.06+.2*q)*gsin_c(ph)
        c.wave(lambda t:sine(t,1.5),amp=2,mid=8-sep//2,col=TRACK)
        c.wave(lambda t:sine(t+wob,1.5),amp=2,mid=8-sep//2+sep)
        if active:dot(24,8-sep//2+sep-2*sine(1+wob,1.5))
    elif gid=='PHASER':
        baseline();w=1+3*q;off=rnd((gsin_c(ph))*3) if active else 0
        xs=(7+off,17+off)
        pts=[(1,4)]
        for x in xs:pts+=[(x-w,4),(x,4+11*(.4+.6*q)),(x+w,4)]
        pts.append((24,4));c.line(pts)
    elif gid in ('FEEDBACK','DELAY_SEND'):tapset('feedback' if gid=='FEEDBACK' else 'send')
    elif gid=='REVERB_SEND':
        # Dense diffuse tail (many close reflections) vs DELAY_SEND (few spaced taps).
        baseline();c.rect(1,3,2,13,GRAY)
        for j in range(10):
            h=rnd(13*q*gexp(-j/5.0))
            if h>=1:c.rect(5+j*2,16-h,1,h)
        if active:dot(5+18*p,16-rnd(13*q*gexp(-9*p/5.0)))
    elif gid=='DELAY_TIME':
        baseline();gap=rnd(3+7*q)
        c.rect(1,3,2,13)
        x=1+gap
        while x<24:
            c.rect(x,8,1,8);x+=gap
        c.line([(1,17),(1+gap,17)],GRAY)
        if active:dot(1+23*p,16)
    elif gid=='TONE':
        baseline();v=2*q-1
        c.line([(1,9+rnd(5*v)),(8,9+rnd(5*v)),(17,9-rnd(5*v)),(24,9-rnd(5*v))])
        c.line([(12,4),(12,14)],TRACK)
    elif gid in ('TAPE','DELAY_TYPE'):
        if gid=='DELAY_TYPE' and s==0:
            baseline()
            for j in range(4):c.rect(2+j*7,3+j*3,2,13-j*3)
        else:
            tq=q if gid=='TAPE' else .5
            a=live.get('tape_phase',0) if (gid=='TAPE' and live.get('tape_active')) else 0
            for cx in (6,19):
                c.circle(cx,7,5);c.rect(cx,7,1,1)
                for k in range(3):
                    u=a+k/3.0;c.line([(cx+2*gsin_c(u+.25),7+2*gsin_c(u)),(cx+3.5*gsin_c(u+.25),7+3.5*gsin_c(u))])
            c.line([(1,8),(1,14),(24,14),(24,8)],TRACK)
            if gid=='TAPE':c.rect(2,16,1+rnd(21*tq),1)
    elif gid=='WEAR':
        c.line([(1,5),(24,5)],TRACK);c.line([(1,13),(24,13)],TRACK)
        gaps=[5,11,17,21,8,14][:rnd(6*q)]
        for x in range(1,25):
            if x not in gaps:c.rect(x,7,1,5)
    elif gid=='PINGPONG':
        c.line([(1,5),(24,5)],TRACK);c.line([(1,13),(24,13)],TRACK)
        for j in range(4):c.rect(2+7*j,(3 if j%2==0 or s==0 else 11),2,5)
        if s:c.line([(3,8),(9,10),(16,8),(23,10)],GRAY)
    elif gid in ('THRESHOLD','RATIO','MAKEUP','COMPRESSOR','LIMITER'):
        baseline();c.line([(2,16),(23,2)],TRACK)
        th=.2+.65*q if gid=='THRESHOLD' else .5
        ra=1+9*q if gid=='RATIO' else 4
        mg=.35*q if gid=='MAKEUP' else 0
        def comp(t):
            if gid=='LIMITER':return min(t,.45) if s else t
            if gid=='COMPRESSOR':t0=.8-.55*q;r=1+5*q
            else:t0=th;r=ra
            return clamp((t if t<t0 else t0+(t-t0)/r)+mg)
        c.line([(2+i*21/64,16-14*comp(i/64)) for i in range(65)])
        if gid=='THRESHOLD':y=rnd(16-14*th);c.line([(2,y),(23,y)],GRAY)
        if gid=='LIMITER' and s:y=rnd(16-14*.45);c.line([(2,y),(23,y)],GRAY)
        if gid=='MAKEUP':
            mg0=mg;mg=0;c.line([(2+i*21/64,16-14*comp(i/64)) for i in range(65)],GRAY);mg=mg0
        if gid=='COMPRESSOR' and active:dot(2+21*amp,16-14*comp(amp))
    elif gid=='BLEND':
        baseline();hd=rnd(13*(1-q));hw=rnd(13*q)
        c.rect(5,16-hd,4,hd,GRAY);c.rect(16,16-hw,4,hw,INK)
    elif gid=='PUMP':
        baseline();depth=rnd(12*q)
        pts=[(1,3),(4,3),(5,3+depth)]+[(5+i*18/24,3+depth*(1-i/24)**2) for i in range(1,25)]+[(24,3)]
        c.line(pts)
        if active:dot(5+18*p,3+depth*(1-p)**2)
    elif gid=='SOURCE':
        # Mirror of DESTINATION: n selectable sources converge on the one control (right).
        n=ctx.get('count',STATES['SOURCE'])
        for j in range(n):
            y=rnd(2+j*14/max(1,n-1));c.rect(1,y-1,3,3,INK if j==s else TRACK)
            c.line([(4,y),(16,y),(16,9)] ,INK if j==s else TRACK)
        c.line([(16,9),(21,9)]);c.circle(22,9,1,fill=True)
    elif gid=='TEMPO':
        c.line([(4,17),(9,2),(16,2),(21,17),(4,17)],INK)
        playing=bool(live.get('playing'))
        angle=.5*gsin_c(live.get('clock_phase',0)) if playing else 0.0
        # Weight lower on the arm = faster tempo (as on a real metronome).
        wy=rnd(5+7*q);L=15-wy
        x=12.5+12*gsin_r(angle)*(1);y=15-12*gcos_r(angle)
        c.line([(12.5,15),(x,y)],WHITE if playing else GRAY)
        wx=12.5+(15-wy)*gsin_r(angle)/1.0*0+ (x-12.5)*(15-wy)/12;wyy=15-(15-y)*(15-wy)/12
        c.rect(wx-1.5,wyy-1,3,2)
    elif gid=='SWING':
        baseline()
        for x,y,h in [(2,3,13),(rnd(12+8*q),7,9),(23,3,13)]:c.rect(x,y,1,h)
    elif gid=='LENGTH':
        n=1+rnd(q*7)
        for j in range(8):c.rect(1+j*3,6,2,7,INK if j<n else TRACK)
        c.line([(n*3,3),(n*3,16)],GRAY);baseline()
    elif gid=='DENSITY':
        k=rnd(q*8);hits=[(j*k)%8<k for j in range(8)] if k else [False]*8
        hits=[((j*k)//8)!=(((j+1)*k)//8) for j in range(8)]
        for j in range(8):
            if hits[j]:c.rect(1+j*3,5,2,11)
            else:c.rect(1+j*3,15,2,1,TRACK)
        baseline()
    elif gid=='SLOT':
        n=rnd(q*7)
        for j in range(8):c.rect(2+(j%4)*6,3+(j//4)*7,4,5,INK if j==n else TRACK)
    elif gid=='CHANCE':
        c.line([(6,3),(18,3),(18,15),(6,15),(6,3)])
        pip={0:[],1:[(12,9)],2:[(9,6),(15,12)],3:[(9,6),(12,9),(15,12)],4:[(9,6),(15,6),(9,12),(15,12)],5:[(9,6),(15,6),(12,9),(9,12),(15,12)],6:[(9,6),(15,6),(9,9),(15,9),(9,12),(15,12)]}[rnd(q*6)]
        for x,y in pip:c.rect(x-1,y-1,2,2)
    elif gid=='MUTATE':
        base=[11,7,9,4,12,8];k=rnd(q*6);order=[1,4,2,5,0,3]
        for j,y in enumerate(base):
            x=1+j*4;moved=order.index(j)<k
            c.rect(x,y,3,2,TRACK)
            if moved:c.rect(x,y+(-3 if y>8 else 4),3,2,INK)
        baseline()
    elif gid in ('REPEAT','RATE'):
        baseline();count=1+rnd(q*7) if gid=='REPEAT' else 2+rnd(q*6)
        xs=[2+rnd(i*21/max(1,count-1)) for i in range(count)]
        for i,x in enumerate(xs):c.rect(x,5 if gid=='RATE' else 4+i,1,11 if gid=='RATE' else 12-i)
        if gid=='RATE' and active:dot(2+21*p,16)
    elif gid=='DIRECTION':
        baseline()
        if s==0:c.arrow(2,9,23,9)
        elif s==1:c.arrow(23,9,2,9)
        elif s==2:c.arrow(4,9,23,9);c.arrow(21,9,2,9)
        else:c.line([(2,14),(8,4),(14,14)]);c.arrow(14,14,22,5)
    elif gid in ('ROOT','SCALE','KEY_RANGE'):
        # One-octave keyboard: 7 white keys (2 px, 1 px gaps); 5 black keys (3 px, upper half).
        white=[0,2,4,5,7,9,11];black={1:0,3:1,6:3,8:4,10:5}
        if gid=='ROOT':mask=1<<rnd(q*11)
        elif gid=='SCALE':
            scales=[0b111111111111,0b101010110101,0b010110101101,0b011010101101,0b001010010101]
            mask=ctx.get('scale_mask',scales[rnd(q*4)])
        else:
            hi=rnd(q*11);mask=sum(1<<k for k in range(hi+1))
        for i,pc in enumerate(white):c.rect(2+i*3,2,2,14,INK if mask>>pc&1 else TRACK)
        for pc,k in black.items():
            g=4+3*k;c.erase(g-2,2,5,8);c.rect(g-1,2,3,7,INK if mask>>pc&1 else GRAY)
        if gid=='KEY_RANGE':c.line([(1,17),(24,17)],TRACK)
    elif gid=='NOTE':
        for y in (4,9,14):c.line([(2,y),(23,y)],TRACK)
        yy=15-12*q;c.circle(11,yy,2,fill=True)
        c.line([(13,yy),(13,yy-6)] if yy>8 else [(9,yy),(9,yy+6)])
        if active and p<.5:dot(11,yy)
    elif gid=='GATE':
        baseline();end=rnd(3+20*q);c.line([(1,15),(2,15),(2,4),(end,4),(end,15),(24,15)])
        if active:dot(2+21*p,4 if 2+21*p<=end else 15)
    elif gid=='LOCK':
        c.line([(8,9),(8,5),(10,2),(15,2),(17,5),(17,9)] if s else [(10,8),(10,5),(12,2),(17,2),(19,5)])
        c.line([(5,9),(20,9),(20,17),(5,17),(5,9)]);c.rect(12,12,2,3)
    elif gid=='MOTION':
        baseline();pts=[(1,10),(5,10-6*q),(9,10+5*q),(15,10-5*q),(19,10),(24,10-4*q)];c.line(pts)
        c.circle(3,3,1,GRAY,fill=True)
        if active:dot(1+23*p,10)
    elif gid=='LINK':
        dx=0 if s else 2
        c.circle(9-dx,9,4);c.circle(16+dx,9,4)
    elif gid=='OCTAVE':
        baseline();steps=1+s
        pts=[(1,15)];x=1
        for j in range(steps):
            x+=5;pts+=[(x,15-j*3),(x,12-j*3)]
        pts.append((24,12-(steps-1)*3));c.line(pts)
    elif gid=='TRACKING':
        baseline();c.line([(2,16),(23,16-13*q)])
        for x in range(2,24,3):c.line([(x,15),(x,17)],GRAY)
        if active:dot(2+21*p,16-13*q*p)
    elif gid=='PEDAL':
        # Top: a held note (ink) whose ring-out tail (gray) grows as the damper lifts.
        q=float(s);c.rect(2,3,7,2,INK);tail=rnd(15*q)
        if tail:c.rect(9,3,tail,2,GRAY)
        # Bottom: sustain pedal plate hinged at the right, pressed flat at max.
        c.line([(3,16),(23,16)],TRACK);c.line([(4,rnd(15-7*(1-q))),(21,15)]);c.circle(21,15,1,fill=True)
        if active:c.line([(2,7),(2+rnd(21*env),7)],WHITE)
    elif gid=='ENABLE':
        c.line([(1,10),(8,10)]);c.line([(17,10),(24,10)])
        c.circle(8,10,1,fill=True);c.circle(17,10,1,fill=True);c.line([(9,10),(16,10) if s else (15,4)])
    elif gid=='DESTINATION':
        n=s;y=3+n*6
        for j in range(3):c.line([(8,9),(8,3+j*6),(20,3+j*6)],TRACK)
        c.line([(1,9),(8,9),(8,y),(20,y)])
        for j in range(3):c.rect(21,2+j*6,3,3,INK if j==n else TRACK)
    elif gid=='UNISON':
        n=1+rnd(q*3)
        for j in range(n):c.wave(lambda t,j=j:sine(t+j*.04*q,1.5),amp=1,mid=rnd(9-(n-1)*2+j*4))
    elif gid=='MOD_MODE':
        baseline()
        if s==0:c.wave(lambda t:sine(t,2),amp=5)
        elif s==1:c.line([(1,16),(1,3)],GRAY);c.wave(lambda t:sine(t,2),amp=5,x0=3)
        else:c.line([(1,16),(1,3)],GRAY);c.line([(3,14),(7,3),(12,14),(24,14)])
    elif gid=='LEGATO':
        c.rect(2,10,8,3);c.rect(15,6,8,3)
        if s:c.line([(3,5),(7,2),(18,2),(22,4)])
        else:c.line([(4,15),(4,17)],TRACK);c.line([(17,11),(17,14)],TRACK)
    elif gid=='LOOP':
        c.arrow(3,4,23,4)
        if s:c.line([(23,4),(23,14),(3,14),(3,4)]);c.arrow(10,14,3,14)
        else:c.line([(3,4),(3,14),(18,14)],TRACK)
    else:raise AssertionError('unimplemented '+gid)
    image=c.finish()
    if cell=='strip':
        out=Image.new('RGB',(40,24),BG);out.paste(image,(7,2));image=out
    elif cell!='compact':raise ValueError(cell)
    return image.resize((image.width*scale,image.height*scale),Image.Resampling.NEAREST)

def demo_state(frame,gid):
    """Synthetic note/hit/clock fixture, NOT firmware live proof."""
    t=frame/23; amp=gexp(-3*t);phase=frame/24
    return dict(active=True,amp=amp,env=amp,filter_env=amp,phase=phase,progress=t,mod=gsin_r(2*math.pi*phase),tape_phase=phase,tape_active=True,playing=True,clock_phase=phase)

def getfont(size):return ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',size)
def generate():
    files={}; recs=[]
    for p in ('frames/1x','frames/4x','gifs','review'): (ROOT/p).mkdir(parents=True,exist_ok=True)
    for gid in GLYPHS:
        cases=[]
        for q in VALUES:cases.append((f'v{q:03d}',q,{},None))
        for s in range(STATES.get(gid,0)):cases.append((f's{s:02d}',64,{},s))
        if gid in LIVE:
            for f in (0,1,2,5,10,18,23):cases.append((f'live{f:02d}',96,demo_state(f,gid),None))
        for name,q,live,st in cases:
            for cell in ('compact','strip'):
                im=render(gid,q,live,state=st,cell=cell)
                for scale in (1,4):
                    file=f'frames/{scale}x/{gid}-{cell}-{name}.png'
                    im.resize((im.width*scale,im.height*scale),Image.Resampling.NEAREST).save(ROOT/file)
                    files[file]=hashlib.sha256((ROOT/file).read_bytes()).hexdigest()
            recs.append(dict(glyph=gid,case=name,value=q,state=st,live=live))
        if gid in LIVE:
            ims=[render(gid,96,demo_state(f,gid),cell='strip',scale=4) for f in range(24)]
            # Show a real resting interval between the synthetic note/hit fixtures.
            ims.extend([render(gid,96,cell='strip',scale=4)]*6)
            ims[0].save(ROOT/'gifs'/f'{gid}.gif',save_all=True,append_images=ims[1:],duration=50,loop=0,disposal=2)
    json.dump(files,open(ROOT/'reference-hashes.json','w'),indent=2)
    json.dump(recs,open(ROOT/'cases.json','w'),indent=2)
    json.dump(list(GLYPHS.values()),open(ROOT/'glyphs.json','w'),indent=2)
    # Phone sheet: 780px, labels readable after zoom; 5 x 4x cells in a row.
    ids=list(GLYPHS); rw=780;rh=146
    def sheet(sub,title):
        im=Image.new('RGB',(rw,70+len(sub)*rh),BG);d=ImageDraw.Draw(im)
        d.text((14,8),title,font=getfont(23),fill=WHITE)
        d.text((14,40),'0%           25%           50%           75%          100%   (4x real pixels)',font=getfont(16),fill=GRAY)
        for i,gid in enumerate(sub):
            y=70+i*rh;d.line((12,y,rw-12,y),fill=TRACK)
            d.text((14,y+6),gid+('  • LIVE' if gid in LIVE else '  • VALUE / STATE'),font=getfont(20),fill=INK)
            d.text((14,y+33),GLYPHS[gid]['meaning'],font=getfont(18),fill=WHITE)
            for j,q in enumerate(VALUES):im.paste(render(gid,q,scale=4),(16+j*152,y+60))
        return im
    sheet(ids,'G-FINAL — 0.0.26 shared glyph vocabulary').save(ROOT/'G-FINAL-sheet.png')
    for k in range(0,len(ids),8):sheet(ids[k:k+8],f'G-FINAL {k+1}–{min(k+8,len(ids))} / {len(ids)}').save(ROOT/'review'/f'final-{k//8+1:02d}.png')
    # Live comparisons at fixed q; 4x. White is supplied engine state, not wall clock.
    for k in range(0,len(LIVE),8):
        sub=[g for g in ids if g in LIVE][k:k+8];im=Image.new('RGB',(780,50+len(sub)*110),BG);d=ImageDraw.Draw(im)
        d.text((14,10),'LIVE fixture • fixed value • early → late',font=getfont(21),fill=WHITE)
        for i,gid in enumerate(sub):
            y=50+i*110;d.text((14,y),gid,font=getfont(17),fill=INK)
            for j,f in enumerate((0,2,5,10,23)):im.paste(render(gid,96,demo_state(f,gid),scale=4),(16+j*152,y+24))
        im.save(ROOT/'review'/f'live-{k//8+1:02d}.png')
    # Full-screen placement sample (not a redesigned UI).
    sample=Image.new('RGB',(240,240),BG);d=ImageDraw.Draw(sample);d.text((8,8),'SHARED 26 x 19',font=getfont(13),fill=WHITE)
    for i,gid in enumerate(['CUTOFF','RESONANCE','ATTACK','DECAY','TUNE','LEVEL','PAN','WAVE','LFO_RATE','TAPE','GLIDE','FEEDBACK']):
        x=5+(i%4)*59;y=42+(i//4)*62
        d.text((x,y),gid[:9],font=getfont(8),fill=GRAY);sample.paste(render(gid,64),(x+13,y+15))
    sample.save(ROOT/'G-FINAL-240.png');sample.resize((960,960),Image.Resampling.NEAREST).save(ROOT/'G-FINAL-240-4x.png')
    report=test();json.dump(report,open(ROOT/'self-test.json','w'),indent=2)
    print(json.dumps(report,indent=2))

def test():
    fails=[];valuecounts={};livecounts={};bounds=0
    for gid in GLYPHS:
        hashes=[]
        for q in VALUES:
            im=render(gid,q);hashes.append(hashlib.sha256(im.tobytes()).hexdigest());bounds+=1
            assert im.size==(26,19)
            # Outer pixel rim must remain background (AA may touch immediate neighbour).
        distinct=len(set(hashes));valuecounts[gid]=distinct
        expected=min(5,STATES.get(gid,128))
        if distinct<expected:fails.append(f'{gid}: {distinct}/{expected} value geometries')
        for st in range(STATES.get(gid,0)):
            render(gid,64,state=st);bounds+=1
        if gid in LIVE:
            hs=[hashlib.sha256(render(gid,96,demo_state(f,gid)).tobytes()).hexdigest() for f in range(24)]
            n=len(set(hs));livecounts[gid]=n
            if n<2:fails.append(gid+': static live fixture')
        # Test all 128 controls, proving safe at every intermediate, not just snapshots.
        for q in range(128):render(gid,q);bounds+=1
    return dict(glyphs=len(GLYPHS),release_derived=sum(not x['new'] for x in GLYPHS.values()),new=sum(x['new'] for x in GLYPHS.values()),bounds_cases=bounds,value_distinct=valuecounts,live_distinct=livecounts,failures=fails,status='PASS' if not fails else 'FAIL',scope='Host renderer only; firmware parity, DSP wiring and human semantic recognition NOT_RUN')

if __name__=='__main__':
    a=argparse.ArgumentParser();a.add_argument('--all',action='store_true');a.add_argument('--glyph',choices=list(GLYPHS));a.add_argument('--value',type=int,default=64);a.add_argument('--out',default='glyph.png');a.add_argument('--test',action='store_true');args=a.parse_args()
    if args.all:generate()
    elif args.test:print(json.dumps(test(),indent=2))
    elif args.glyph:render(args.glyph,args.value,scale=4).save(args.out)
    else:a.print_help()
