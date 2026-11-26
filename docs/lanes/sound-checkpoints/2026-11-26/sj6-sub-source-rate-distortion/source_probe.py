"""In-memory rate/distortion of a pinned SuperJ6 source bank. No media output."""
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import sys
import zlib
import numpy as np
from scipy.signal import resample_poly

SFZ_PIN = '3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
DATA_PIN = 'ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
FACTORS = (1,2,3,4,6,8,12,16,24,32,48,64)
BITS = (0,8,12,16) # 0 reports band removal without a quantization/storage claim.
SCREEN_DB = (-30,-40,-50,-60)
CAPACITY_B = (8192,16*1024,24*1024,112*1024)

def shasum(x):
    return hashlib.sha256(x).hexdigest()

def read(sfz_path, data_path):
    sfz,data = sfz_path.read_bytes(),data_path.read_bytes()
    assert shasum(sfz)==SFZ_PIN and shasum(data)==DATA_PIN
    assert data[:8] == b'AKJ6PCM1' and struct.unpack_from('<II',data,8)==(1,12)
    pos = 16
    decoded = {}
    for _ in range(12):
        n, = struct.unpack_from('<H',data,pos)
        pos += 2
        filename = data[pos:pos+n].decode().split('/')[-1]
        pos += n
        frames,rate,off,size,crc = struct.unpack_from('<IIQQI',data,pos)
        pos += 28
        assert size==frames*2 and len(data)>=off+size and filename not in decoded
        # Full container SHA authenticates bytes. Avoid assuming CRC semantics.
        decoded[filename] = (rate,np.frombuffer(data,dtype='<i2',count=frames,offset=off))
    rows = []
    fields = {}
    for line in sfz.decode().splitlines():
        kv = dict(re.findall(r'(\w+)=(\S+)',line))
        if line.startswith('<group>'):
            fields = kv
        elif line.startswith('<region>'):
            q = {**fields,**kv}
            k = int(q['pitch_keycenter'])
            start,stop = [int(q[v]) for v in ('loop_start','loop_end')]
            filename = q['sample'].replace('\\','/').split('/')[-1]
            rate,x = decoded[filename]
            assert rate == 44100 and 0<start<stop<len(x)
            rows.append((k,rate,x,start,stop))
    assert [k for k,*_ in rows] == list(range(30,97,6))
    return rows

def db(a):
    return float(20*np.log10(max(float(a),1e-100)))

def score(a,b,windows):
    result = {}
    for k,(i,j) in windows.items():
        z,y = a[i:j],b[i:j]
        n = np.linalg.norm(y)
        assert n > 0
        result[k] = [db(np.linalg.norm(z-y)/n),db(np.linalg.norm(z)/n)]
    return result

def encode_payload(words, bits):
    if bits==8:
        return words.astype('i1').tobytes()
    if bits==16:
        return words.astype('<i2').tobytes()
    x = (words.astype('i4') & 4095).astype('u2')
    if len(x)%2:
        x = np.pad(x,(0,1))
    a,b=x[0::2],x[1::2]
    out=np.column_stack([a & 255, ((a>>8) | ((b&15)<<4)), b>>4]).astype('u1')
    return out.tobytes()[:math.ceil(len(words)*12/8)]

def run(sfz_path,data_path):
    zones = {}
    sizes = {}
    for key,rate,raw,lo,hi in read(sfz_path,data_path):
        x=raw.astype('f8')/32768.
        guard=round(10*rate/(220*2**((key-69)/12)))
        windows={'strike-0-50ms':(0,rate//20),'early-body-100-500ms':(rate//10,rate//2),
                 'blind-loop-head':(lo+guard,lo+2*guard),'blind-loop-tail':(hi-2*guard,hi-guard),
                 'actual-loop-boundary-before':(hi-256,hi),'actual-loop-boundary-after':(lo,lo+256),
                 'whole':(0,len(x))}
        v = {}
        for f in FACTORS:
            d = x if f==1 else resample_poly(x,1,f,window=('kaiser',9.0),padtype='line')
            for bits in BITS:
                label = f'{f}:{bits}'
                if bits:
                    scale = 2**(bits-1)
                    w = np.clip(np.rint(d*scale),-scale,scale-1)
                    rawbytes=encode_payload(w,bits)
                    y=w/scale
                    byte_limits={'packed':len(rawbytes),'zlib':len(zlib.compress(rawbytes,9))}
                    assert len(rawbytes)==math.ceil(len(d)*bits/8)
                else:
                    y=d
                    byte_limits={}
                if f!=1:
                    y=resample_poly(y,f,1,window=('kaiser',9.0),padtype='line')[:len(x)]
                metrics=score(y,x,windows)
                v[label]={'storage_B':byte_limits,'worst_signed_dbr':max(e[0] for e in metrics.values()),
                          'windows_signed_dbr_and_level_db':metrics}
                if f==1 and bits==16:
                    assert np.array_equal(x,y)
                if bits:
                    size=sizes.setdefault(label,dict(packed=0,zlib=0))
                    for mode,b in byte_limits.items():
                        size[mode]+=b
        zones[key]=v
    pass_min = {}
    for target in SCREEN_DB:
        choices = {}
        for key,v in zones.items():
            q=[(label,o['storage_B'],o['worst_signed_dbr']) for label,o in v.items()
               if o['storage_B'] and o['worst_signed_dbr']<=target]
            selected={}
            for fmt in ['packed','zlib']:
                if q:
                    selected[fmt]=min(q,key=lambda e: e[1][fmt])
            choices[key]=selected
        for fmt in ['packed','zlib']:
            if all(v for v in choices.values()):
                b=sum(v[fmt][1][fmt] for v in choices.values())
                pass_min.setdefault(str(target),{})[fmt]={'payload_B':b,'choices':{k:v[fmt][0] for k,v in choices.items()}}
    return dict(source_keys=list(zones),static_capacity_comparisons_B=CAPACITY_B,all_zone_uniform_payload_B=sizes,
                min_per_zone_selections=pass_min, zones=zones, unqualified=True,
                scope='actual original source media, data pin and map pin; host offline polyphase only, no target decode/timing, typed owner replay or generated media exported')
if __name__=='__main__':
    print(json.dumps(run(*map(Path,sys.argv[1:])),indent=2,allow_nan=False))
