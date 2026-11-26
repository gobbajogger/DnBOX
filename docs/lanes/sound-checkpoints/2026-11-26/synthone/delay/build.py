"""Build authenticated original-source graph with optional bounded delay interposer."""
from pathlib import Path
import json,subprocess,sys,hashlib,os
D=Path(__file__).resolve().parent; W=D.parent
B=W/'s1-fx-parity/reference-build'
P=Path(sys.argv[1]).resolve();P.mkdir(mode=0o700,exist_ok=True)
base=B/'factory-inputs/tests/fixtures/synthone-factory/fullfx/source_fx.hpp'
original=W/'s1-fx-parity/original.cpp'
s=base.read_text()
s=s.replace('struct SourceFx {', 'struct SourceFx {')
point='        // PING PONG DELAY'
j=s.index(point); start=s.index('        delayL->del',j); end=s.index('\n        // DELAY MIXER',start)
s=s[:start]+'''#if defined(S1_PORTABLE_DELAY)
        s1_delay_tap(parameters[delayTime],parameters[delayFeedback],delayInputLowPassOutL,
                     delayInputLowPassOutR,&delayOutL,&delayOutRR);
#else
'''+s[start:end]+'''#endif
'''+s[end:]
s=s.replace('        // DELAY MIXER','''        // Observation-only tap for delay preservation diagnostics.
        static const char* capture_path=std::getenv("S1_DELAY_TAP_TRACE");
        static std::FILE* capture=capture_path?std::fopen(capture_path,"wb"):nullptr;
        if(capture){
            const float tape[]={parameters[delayTime],parameters[delayFeedback],
                delayInputLowPassOutL,delayInputLowPassOutR,delayOutL,delayOutRR};
            std::fwrite(tape,sizeof(float),6,capture);
        }
#if !defined(S1_PORTABLE_DELAY)
        static const char* write_path=std::getenv("S1_FEEDBACK_TAP");
        static std::FILE* write_capture=write_path?std::fopen(write_path,"wb"):nullptr;
        if(write_capture){
            const float tape[]={parameters[delayTime],parameters[delayFeedback],
                delayInputLowPassOutL,delayInputLowPassOutR,delayOutL,delayOutR,
                delayFillInOut,delayOutRR};
            std::fwrite(tape,sizeof(float),8,write_capture);
        }
#endif
        // DELAY MIXER''')
s=s.replace(' void source_reset_delays(){',''' void source_reset_delays(){
#if defined(S1_PORTABLE_DELAY)
    s1_delay_reset();
#endif
''')
s='extern "C" { void s1_delay_tap(float,float,float,float,float*,float*); void s1_delay_reset(void); }\n'+s
(P/'source_fx.hpp').write_text(s)
v='extern "C" void s1_delay_cold(float);\n'+original.read_text()
v=v.replace('    SourceFx fx(S1AB_RATE);', '''#if defined(S1_PORTABLE_DELAY)
    s1_delay_cold(ctrl.target(delayTime));
#endif
    SourceFx fx(S1AB_RATE);''')
(P/'reference.cpp').write_text(v)
cmds=json.loads((B/'factory-build-commands.json').read_text())
t=next(x for x in cmds if '-c' in x and x[x.index('-c')+1].endswith('/preset_audio.cpp'))
l=cmds[-1]
flags=['-DS1_ABLATION'] if os.getenv('S1_ANALYSIS_ABLATION')=='1' else []
subprocess.run(['cc','-std=c11','-O2','-ffp-contract=off',*flags,'-c',str(D/'ring.c'),'-o',str(P/'ring.o')],check=True)
for mode in ('reference','bounded'):
 c=t.copy()
 c[c.index('-c')+1]=str(P/'reference.cpp');c[c.index('-MF')+1]=str(P/f'{mode}.d');c[c.index('-o')+1]=str(P/f'{mode}.o')
 c.extend(['-I'+str(W/'s1-fx-parity')])
 if mode=='bounded':c.append('-DS1_PORTABLE_DELAY=1')
 subprocess.run(c,check=True)
 link=l.copy();link[link.index(str(B/'factory-6.o'))]=str(P/f'{mode}.o');link[link.index('-o')+1]=str(P/mode)
 if mode=='bounded':link.insert(1,str(P/'ring.o'))
 subprocess.run(link,check=True)
print('source_fx_sha256',hashlib.sha256(base.read_bytes()).hexdigest())
