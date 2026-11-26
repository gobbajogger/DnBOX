from pathlib import Path
import sys
p=Path(sys.argv[1]);t=p.read_text()
a='    unsigned x=fold_index(row&3u,p,&sa), y=fold_index(row&3u,q,&sb);'
b='''    const unsigned shape=row&3u;
    unsigned x=fold_index(shape,p,&sa), y;
    /* Reuse the first folded sample and sign inside monotonic runs.
     * Inclusive corners, sign flips and wrap fall back to original. */
    if(shape < 2u && ((p&1023u)!=0 && (p&1023u)!=1023u)) {
        y=(p&1024u)?x-1u:x+1u;
        sb=sa;
    } else if(shape==3u && p!=2048u && p!=4095u) {
        y=p<2048u?x+1u:x-1u;
        sb=sa;
    } else y=fold_index(shape,q,&sb);'''
assert t.count(a)==1
p.write_text(t.replace(a,b))
