"""Same arithmetic as verified CoreSampler .37af88 and .37b378–.37b400."""
from struct import pack,unpack
import json
f=lambda v:unpack('<f',pack('<f',v))[0]
def model(t):
    g=1.;mode=0;request=0
    rate=t['rate'];step=f(1./max(1.,f(rate*f(.005))))
    parts=t['parts'];events={int(a):v for a,v in t['events'].items()}
    edges=[[0,0]];guards=[];gains=[];mark=0;i=0
    while mark<1024:
        if mark in events:request=events[mark]
        pending=[u for u in events if u>mark]
        n=min(parts[i%len(parts)],min([1024]+pending)-mark)
        start=mark
        while mark<start+n:
            block=min(16,start+n-mark)
            if mode!=request and g<=0.:
                mode=request;edges.append([mark,mode]);guards.append([start,mode,64])
            if request!=mode or g<1.:
                for _ in range(block):
                    g=max(0.,f(g-step)) if request!=mode else min(1.,f(g+step))
                    gains.append(g)
            mark+=block
        i+=1
    return edges,guards,gains
r=json.load(open('fade.json'))
for t in r['trials']:
    a=[e[:2] for e in t['edges']]
    m,g,floats=model(t)
    assert a==m,(t['rate'],t['events'],t['parts'],a,m)
    assert g==t['resets_by_callback']
    assert floats[:5]==t['first_gains']
    assert sorted(floats)[:5]==t['low_gains']
    assert len(floats)==t['fade_count']
    assert all(not any(bytes.fromhex(e[2])) for e in t['edges'][1:])
print('Exact original all 16 model vs actual edge/gain-count/reset callback assertions PASS; selected pre-first tap history zero')
