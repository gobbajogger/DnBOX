from pathlib import Path
import hashlib,sys
a,b=map(Path,sys.argv[1:])
r=a.read_bytes()
assert hashlib.sha256(r).hexdigest()=='c175a5d50a161fcc0d4eb7dade04b6c6205d2243bd241a768ff93902e0d63def'
s=r.decode()
q='''            sj6_crush_process(p,left,right,frames);'''
assert s.count(q)==1
s=s.replace(q,'''            extern int sj6_validated_dry_pre(float*,float*,unsigned);
            if(!sj6_validated_dry_pre(left,right,frames))
'''+q)
q='''                right[i]*=gain;
            }
}'''
assert s.count(q)==1
s=s.replace(q,'''                right[i]*=gain;
            }
            extern void sj6_validated_dry_post(float*,float*,unsigned);
            sj6_validated_dry_post(left,right,frames);
}''')
b.write_text(s)
