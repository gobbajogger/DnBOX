from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='static uint32_t lab_sub_pcm'
assert s.count(a)==1
s=s.replace(a,'''static const float *sub_tape;
static unsigned tape_frames, active_frames;
static float *diag_stock;
static unsigned frame_clock;
__attribute__((visibility("default")))
void bind_original_sub(const float *tape,unsigned n,float *stock){
    sub_tape=tape;tape_frames=n;active_frames=frame_clock=0;diag_stock=stock;
}
__attribute__((visibility("default")))
unsigned audited_sub_frames(void){return active_frames;}
static uint32_t lab_sub_pcm''')
a='''                    if(audible_main_amp==amp)mix[l]+=velocity*amp*y;
                    else mix[l]+=velocity*(amp*sub_y+audible_main_amp*main_y);'''
assert s.count(a)==1
s=s.replace(a,'''                    if(diag_stock){
                        assert(frame_clock<tape_frames);
                        diag_stock[frame_clock]+=(velocity*amp*sub_y)*p->gain;
                    }
                    if(sub_tape){
                        assert(ota_arm==4 && l==0 && p->gain==1.f);
                        mix[l]+=velocity*(audible_main_amp*main_y);
                    }else if(audible_main_amp==amp)mix[l]+=velocity*amp*y;
                    else mix[l]+=velocity*(amp*sub_y+audible_main_amp*main_y);''')
a='''            left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);
        }
    }'''
assert s.count(a)==1
s=s.replace(a,'''            left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);
        }
        if(sub_tape){
            assert(use_source && frame_clock<tape_frames);
            left[k]+=sub_tape[frame_clock];
            right[k]+=sub_tape[tape_frames+frame_clock];
            ++active_frames;
        }
        ++frame_clock;
    }''')
p.write_text(s)
