from pathlib import Path
import sys
r=Path(sys.argv[1]).resolve();out=Path(sys.argv[2])
d=r/'tests/host'
s=(d/'dual-cable.c').read_text().replace('#include "native-test-body.inc"',f'#include "{d}/native-test-body.inc"')
h=(r/'firmware/hal/smk_input.h').read_text()
a=h.index('static void smk__padq(');z=h.index('\n}',a)+2
p=h.index('static const uint8_t SMK_WHITE_NOTE');q=h.index('\n#define SMK_TRIM_FLASH',p)
s=s.replace('static int hex_digit', '''
#define SMK_NPAD 16u
#define SMK_KEY_F3 5u
static struct { uint32_t notes,knotes,pnotes,notes_pressed; uint8_t pad_vel[16],key_vel[37]; } fm1_in;
'''+h[p:q]+'\n'+h[a:z]+'''
static int digital_pad(unsigned pad,unsigned velocity){
 if(pad>=16||velocity>127)return 0;
 smk__padq(pad,velocity);
 held_keys=fm1_in.notes;
 for(int i=0;i<NKEYS;i++)key_velocity[i]=fm1_in.key_vel[SMK_KEY_F3+i];
 tick(10);return 1;
}
static int hex_digit''',1)
s=s.replace('  while(fgets(line,sizeof line,stdin)){','''
  ui.part=PART_303A; set_view(V_PART);build_pages();tick(25);
  while(fgets(line,sizeof line,stdin)){
    if(line[0]=='H'){
      unsigned p,v;if(sscanf(line+2,"%u%u",&p,&v)!=2||!digital_pad(p,v))return 9;
      puts("END");fflush(stdout);continue;
    }''',1)
out.write_text(s)
