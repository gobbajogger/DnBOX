"""Exact two-hunk host-only production ingress observer; no audio writes."""
from pathlib import Path
import sys,hashlib
a,b=map(Path,sys.argv[1:])
p=Path(__file__).resolve().parent
pins={name:h for h,name in (line.split() for line in (p/'pins.txt').read_text().splitlines())}
s=a.read_bytes()
assert hashlib.sha256(s).hexdigest()==pins['original_presets.inc']
s=s.decode()
q='''static __attribute__((noinline)) void sj6_crush_choose_store(unsigned owner, unsigned k, unsigned pos)
{
    sj6_crush_write((sound_t*)mot_base_snd,owner)[k]=(uint8_t)pos;
    sj6_crush_apply(mot_base_snd,owner);
}'''
assert s.count(q)==1
head='''#if X0X_HOST && defined(SJ6_CRUSH_EDGE_WITNESS)
extern void sj6_crush_ingress_accepted(unsigned owner,unsigned panel_id,unsigned code,float canonical);
extern void sj6_crush_ingress_revoke(unsigned owner);
#endif
'''
s=s.replace(q,head+q.replace('    sj6_crush_apply(mot_base_snd,owner);','''    sj6_crush_apply(mot_base_snd,owner);
#if X0X_HOST && defined(SJ6_CRUSH_EDGE_WITNESS)
    /* Project hold remains active in original_typed_indexed_choose. */
    sj6_crush_ingress_accepted(owner,107u+k,k?pos:pos*127u,k?unit_control(pos):(float)pos);
#endif'''))
q='''X0X_COLD void sj6_native_room_undo(unsigned mask)
{
    original_typed_undo_apply(mask,sj6_native_room_apply);
}'''
assert s.count(q)==1
s=s.replace(q,q.replace('    original_typed_undo_apply', '''#if X0X_HOST && defined(SJ6_CRUSH_EDGE_WITNESS)
    for(unsigned owner=0;owner<NBASS;owner++)
        if(mask&(1u<<owner))sj6_crush_ingress_revoke(owner);
#endif
    original_typed_undo_apply'''))
b.write_text(s)
