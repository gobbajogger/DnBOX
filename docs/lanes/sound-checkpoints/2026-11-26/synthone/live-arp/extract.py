from pathlib import Path
import hashlib,sys
d=Path(__file__).resolve().parent
patched="--patched" in sys.argv
r=d.parent
a=r/'s1-organ-wet-recovery/native-widen-bfp/tree/firmware/src/app'
p=(a/'original_runtime.c')
assert hashlib.sha256(p.read_bytes()).hexdigest()=='35efa9b84fb207b0f905771d3d0007beb7a51ca5eb1a27a4570a03505c5e8a4b'
if patched:
 p=d/'private/full-tree/firmware/src/app/original_runtime.c'
s=p.read_text()
def take(first,after):
 i=s.index(first);j=s.index(after,i+len(first)); return s[i:j]
snip=[
 take('void engine_arp_cfg_default(', 'static uint8_t arp_clamp('),
 take('static int arp_round(', '/* SuperJ6'), 
 take('int engine_arp_from_s1(', '/* ---- Shared Synth One'),
 take('static void arp_hold(', '/* engine_arp_cfg ->'), 
 take('#if X0X_SUPERJ6_BUILTIN_INIT\n/* Build-9 Saga periods', 'static const uint8_t arp_ticks['),
 take('static const uint8_t arp_ticks[', '#if X0X_SJ6_AUX_SEQ_NATIVE'),
 take('uint32_t engine_arp_step_frames(', 'void original_runtime_arp_clock('),
]
# Excerpt leaves the actual runtime functions and shared core source unmodified.
(d/('private/patched-include/arp-runtime.inc' if patched else 'private/arp-runtime.inc')).write_text(''.join(snip))
orig=r/'recovered-sound/s1-room-parity/unified-releasephase/orig.cpp'
o=orig.read_text()
marker='    CHECK(!std::fclose(f));o.resetSequencer();'
assert o.count(marker)==1
o=o.replace(marker,marker+'''
    if(const char *path=std::getenv("S1_ORACLE_ARP_STATE")){
        FILE *state=std::fopen(path,"w");CHECK(state);
        for(unsigned id=60;id<=114;++id)std::fprintf(state,"%u %a\\n",id,double(o.parameters[id]));
        std::fprintf(state,"147 %a\\n",double(o.parameters[147]));
        CHECK(!std::fclose(state));
    }
''')
(d/'private/orig.cpp').write_text(o)
