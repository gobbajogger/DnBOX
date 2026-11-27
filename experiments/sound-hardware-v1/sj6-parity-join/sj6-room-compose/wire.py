from pathlib import Path
import sys,hashlib
src,out=map(Path,sys.argv[1:])
srcs={'original_runtime.c':'0bc8debd1fa37d8e45be99fe0ee0cb8b081e96d70fcc31834905a80d9aadee8a',
      'original_runtime.h':'a65b8a008fb8b953b810d462cbc2295cade924c9220657c54d9af94a048cea33'}
g='#if defined(X0X_HOST) && defined(DNBOX_ROOM_COMPOSE_LAB)'
s=(src/'original_runtime.h').read_text();assert hashlib.sha256(s.encode()).hexdigest()==srcs['original_runtime.h']
key='    original_runtime_backend dsp;'
assert s.count(key)==1
s=s.replace(key,key+'\n'+g+'\n    struct RoomOwner *lab_room;\n    const struct RoomBlock *lab_room_block;\n#endif')
(out/'original_runtime.h').write_text(s)
s=(src/'original_runtime.c').read_text();assert hashlib.sha256(s.encode()).hexdigest()==srcs['original_runtime.c']
key='#include "../dsp/sj6_native_room.inc"';assert s.count(key)==1
s=s.replace(key,key+'\n'+g+'\n#include "room.h"\n#endif')
key='''            sj6_crush_process(p,left,right,frames);
            sj6_room_process(&p->dsp.juno.room,left,right,frames);'''
assert s.count(key)==1
s=s.replace(key,g+'''
            if(p->lab_room_block){
                /* Input: explicit separate dry-Rage and post wet HP witness.
                 * Not a mapping from compact normalized fields or a product fallback. */
                compose_room_tick(p->lab_room,p->lab_room_block,left,right,frames);
            } else
#endif
            {
'''+key+'''
            }''')
# No added production branch or state; retain braces only inside lab preprocessor output.
s=s.replace('''            } else
#endif
            {
''','''            } else {
#endif
''').replace(key+'\n            }',key+'\n'+g+'\n            }\n#endif')
(out/'original_runtime.c').write_text(s)
