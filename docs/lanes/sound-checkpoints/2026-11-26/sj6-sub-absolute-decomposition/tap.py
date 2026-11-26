from pathlib import Path
import hashlib,sys
inp,core,driver,out,tape=map(Path,sys.argv[1:])
assert hashlib.sha256(inp.read_bytes()).hexdigest()=='fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e'
s=inp.read_text()
a='        float mix[2]'
assert s.count(a)==1
s=s.replace(a,'        extern void wordtap(unsigned, float, float, float);\n        extern void endtap(float, float, float);\n'+a)
a='                mix[l]+=velocity*amp*lv->filter_second;'
assert s.count(a)==1
s=s.replace(a,a+'''
                if(i==0&&!l&&s->sj6_sub_gain)
                    wordtap(k,x,(velocity*amp)*x,(velocity*amp)*lv->filter_second);
''')
a='            left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);\n        }'
assert s.count(a)==1
s=s.replace(a,a+'\n        endtap(mix[0],left[k],right[k]);')
core.write_text(s)
s=driver.read_text()
a='static unsigned samples;'
s=s.replace(a,a+f'''
static FILE* tap;
static float record[6];
void wordtap(unsigned k,float x,float pre,float post) {{
    (void)k;
    record[0]=x; record[1]=pre; record[2]=post;
}}
void endtap(float sum,float l,float r) {{
    record[3]=sum; record[4]=l;record[5]=r;
    assert(fwrite(record,sizeof(float),6,tap)==6);
    memset(record,0,sizeof(record));
}}
''')
a='  prepare(bpm);'
s=s.replace(a,f'  tap=fopen("{tape}","wb"); assert(tap);\n'+a)
out.write_text(s)
