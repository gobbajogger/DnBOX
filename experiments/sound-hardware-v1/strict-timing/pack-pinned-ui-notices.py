"""Experimental exact notice packing for one pinned donor; no DSP calls."""
from pathlib import Path
import hashlib,re,sys
d,elf=map(Path,sys.argv[1:])
p=d/'tree/firmware/src/app/ui.c'
src=p.read_bytes(); raw=elf.read_bytes()
sha=lambda x:hashlib.sha256(x).hexdigest()
assert sha(src)=='ec1f789a5ad3f3e4928b86c3b78f244ef23aad2a027c22177c60b7e6a192a698'
assert sha(raw)=='e0baf31f90a427077c08181067a02e4eb05e504cfbdca3a696966ff0be2eb306'
s=src.decode()
targets=set(re.findall(r'\bsay\s*\(\s*"([^"\\]*)"',s))|set(re.findall(r'"(UNDO [^"\\]+)"',s))
msgs=sorted(x for x in targets if x and x.encode()+b'\0' in raw)
alphabet=sorted(set(''.join(msgs)))
phrases=[' INVALID','UNDO ','WAIT ','UNAVAILABLE','SOUND','DELAY',' SAVE','LFO','MOTION ','CLEARED','TE','SE','PROJECT ','AN',' NOT',' SLICE','WET MAKEUP','NEWER DATA:','LOCK','NG ','LE','ITCH','IN CUTOFF','CHOR']
tokens=['']+alphabet+phrases
slots={x:i for i,x in enumerate(tokens)}
assert len(msgs)==104 and len(alphabet)==33 and len(set(tokens))==len(tokens)
def encode(text):
    chars=list(text)
    for ph in phrases:
        q=list(ph);n=len(q);i=0
        while i+n<=len(chars):
            if chars[i:i+n]==q:chars[i:i+n]=[ph]
            i+=1
    keys=[slots[t] for t in chars]+[0]
    return sum(k<<(i*6) for i,k in enumerate(keys)).to_bytes((len(keys)*6+7)//8,'little')
def decode(p):
    v=int.from_bytes(p,'little');res=''
    for i in range((len(p)*8)//6):
        c=(v>>(6*i))&63
        if not c:return res
        res+=tokens[c]
    raise AssertionError("missing EOS")
blob=b'';dictionary=b''; offsets=[]
for ph in phrases:
    offsets.append(len(dictionary));dictionary+=ph.encode('ascii')+b'\0'
for msg in msgs:
    e=encode(msg)
    assert decode(e)==msg
    off=len(blob);blob+=b'\xff'+e
    rep=f'((const char *)(fixed_ui_notices+{off}))'
    lit='"'+msg+'"'
    if msg.startswith('UNDO '):s=s.replace(lit,rep)
    else:s=re.sub(r'\bsay\(\s*'+re.escape(lit),'say_fixed('+rep,s)
s=re.sub(r'\bsay\(([^;]{0,500}fixed_ui_notices[^;]{0,300})\);',r'say_fixed(\1);',s)
assert s.count('say(message,0);')==3
s=s.replace('say(message,0);','say_fixed(message,0);')
assert len(blob)==811 and len(dictionary)==156 and len(offsets)==24
def arr(name,values):
    st=f'static const unsigned char {name}[]={{\n'
    for at in range(0,len(values),24):st+=','.join(map(str,values[at:at+24]))+',\n'
    return st+'};\n'
out=arr('fixed_ui_notices',blob)+arr('fixed_ui_dictionary',dictionary)+arr('fixed_ui_offsets',offsets)+arr('fixed_ui_alphabet',list(''.join(alphabet).encode('ascii')))
out+='''
__attribute__((noinline)) static void say_fixed(const char *a,const char *b)
{
    const unsigned char *p=(const unsigned char *)a;
    if(*p!=255u){say(a,b);return;}
    ++p;char *q=ui.msg;unsigned acc=0,nbit=0;
    for(;;){
        if(nbit<6){acc|=(unsigned)*p++<<nbit;nbit+=8;}
        unsigned t=acc&63u;acc>>=6;nbit-=6;if(!t)break;
        if(t<=33)*q++=(char)fixed_ui_alphabet[t-1];
        else q=put_s(q,(const char*)fixed_ui_dictionary+fixed_ui_offsets[t-34]);
    }
    *q=0;
    if(b)put_s(q,b);
    ui.msg_until=plat_ms()+1300u;
}
'''
anchor='/* =============================================================== param refs === */'
assert s.count(anchor)==1
s=s.replace(anchor,out+'\n'+anchor)
p.write_text(s)
print('verified',len(msgs),'fixed notices, max literal',max(map(len,msgs)), 'bytes',len(blob),len(dictionary), 'output',sha(p.read_bytes()))
