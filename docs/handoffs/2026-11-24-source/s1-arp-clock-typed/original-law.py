from pathlib import Path
from sys import argv
p=Path(argv[1])/'firmware/src/app'
def edit(which,pairs):
 f=p/which;s=f.read_text()
 for a,b in pairs:
  assert s.count(a)==1,(which,a)
  s=s.replace(a,b)
 f.write_text(s)
edit('original_presets.inc',[
 ('return p[2] ? p[2]-1u : 5;', 'return p[2] ? p[2]-1u : 9;'),
 ('p[2] ? p[2] : 6;', 'p[2] ? p[2] : 10;')])
edit('ui.c',[
 ('''    static const char *const stops[]={"8 BARS","6 BARS","4 BARS","3 BARS","2 BARS","1 BAR","1 BAR T",
        "1/2","1/2 T","1/4","1/4 T","1/8","1/8 T","1/16","1/16 T","1/32","1/32 T","1/64","1/64 T"};''',
 '''    static const char *const stops[]={"1/64 T","1/64","1/32 T","1/32","1/16 T","1/16","1/8 T","1/8",
        "1/4 T","1/4","1/2 T","1/2","1 BAR T","1 BAR","2 BARS","3 BARS","4 BARS","6 BARS","8 BARS"};'''),
 ('"CLOCK FACTOR",18,5,stops','"CLOCK FACTOR",18,9,stops'),
 ('return value<0 ? 0 : value-(field==ARP_F_SOURCE_RATE);',
  'return value<0 ? 0 : field==ARP_F_SOURCE_RATE ? value-1 : 18-value;'),
 ('engine_arp_source_set(voice,field,v+(field==ARP_F_SOURCE_RATE))',
  'engine_arp_source_set(voice,field,field==ARP_F_SOURCE_RATE ? v+1 : 18-v)'),
 ('    int switched=original_switch_turn(r,pref_get(r),e,1);',
 '''#if X0X_S1_FENCED_CLOCK
    if (r.kind==R_ARP && (r.a==ARP_F_SOURCE_RATE || r.a==ARP_F_SOURCE_FACTOR)) {
        // Source stepper buttons and factor stops advance one position per detent.
        if(e) pref_set_impl(r,pref_get(r)+(e>0?1:-1),!accel_smc);
        return;
    }
#endif
    int switched=original_switch_turn(r,pref_get(r),e,1);''')])
