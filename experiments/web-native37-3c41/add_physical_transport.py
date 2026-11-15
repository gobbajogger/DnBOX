"""Generate host instrumentation from exact existing production SMK functions."""
from pathlib import Path
import hashlib,json
BASE=Path(__file__).resolve().parent;SRC=BASE/'source-restoration/native-3c41'
def function(text,start):
 a=text.index(start);b=text.index('{',a);depth=1;i=b+1
 while depth:
  depth+=(text[i]=='{')-(text[i]=='}');i+=1
 return text[a:i]
h=(SRC/'firmware/hal/smk_input.h').read_text();p=(SRC/'firmware/src/app/plat_fm1.c').read_text();r=(SRC/'tests/host/rec_live_test.c').read_text()
stubs='\n'.join(function(r,name) for name in ('void smk_apc_clock_tick(', 'void smk_apc_clock_start(', 'void smk_apc_clock_position(', 'void smk_apc_transport_led_set(')) # Additional exact disconnected MIDI fixture hooks; theme supplies the others
transport='/* Exact source-extracted production transport, isolated host only. */\n'+stubs+'\n#define SMK_KEY_F3 5u\n#define SMK_EXTQ 32u\nstatic struct { uint32_t notes,knotes,pnotes,notes_pressed; uint8_t key_vel[37]; uint64_t keys; } fm1_in;\n'+h[h.index('static volatile uint32_t smk_extq'):h.index('\nstatic uint8_t fm1_led')]+'\n'+function(h,'static void smk__extq(')+'\n'+function(h,'static void smk__key_event(')+'\nstatic unsigned irq_depth;\nstatic uint32_t irq_save(void){return irq_depth++;}\nstatic void irq_restore(uint32_t f){irq_depth=f;}\n'+function(p,'uint64_t plat_physical_keys(')+'\n'+function(p,'int plat_outer_keys(')+'\n'
m=(SRC/'firmware/src/app/main_fm1.c').read_text();a=m.index('    extern void ui_brk_page_keys',m.index('CK_FN static void smk_frame(void)'));b=m.index('    ui_brk_page_keys(outer, allow_pages);',a)+len('    ui_brk_page_keys(outer, allow_pages);')
transport+='\n#define clip_grid_mode smk_clip_mode_on()\nstatic void host_physical_frame(void) {\n'+m[a:b].replace('!padcal.on','!smk_padcal_on()')+'\n}\n#undef clip_grid_mode\n'
(BASE/'physical-transport.h').write_text(transport)
manifest={'source_pin':'3c41ef4bedf9701f549ce2752b73306e2fab1eb8','authority':'source-extracted production SMK key event and platform queue; host-only IRQ shim/disconnected SMK fixture stubs','files':{str(x):hashlib.sha256((SRC/x).read_bytes()).hexdigest() for x in ['firmware/hal/smk_input.h','firmware/src/app/main_fm1.c','firmware/src/app/plat_fm1.c','firmware/src/app/plat.h','firmware/src/app/ui.c','tests/host/freeplay_live_keys_fixture.py','tests/host/freeplay_live_keys_cases.inc','tests/host/rec_live_test.c']},'generated_sha256':hashlib.sha256(transport.encode()).hexdigest()}
(BASE/'PHYSICAL-TRANSPORT-SOURCE.json').write_text(json.dumps(manifest,indent=2)+'\n')
