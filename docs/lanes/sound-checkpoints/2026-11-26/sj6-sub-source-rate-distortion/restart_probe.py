"""Measure indexed restart overhead before proposing a realtime decoder."""
import json
import sys
from pathlib import Path
import zlib
import numpy as np
from scipy.signal import resample_poly
from source_probe import read, encode_payload

BLOCKS=(256,512,1024,2048,4096,8192,16384,32768)
THRESHOLDS=(-30,-40)

def analyze(sfz,asset,prior):
    r=json.loads(prior.read_text())['min_per_zone_selections']
    cases={}
    for lev in THRESHOLDS:
        pick=r[str(lev)]['zlib']
        rows={}
        for block in BLOCKS:
            payload=meta=entries=0
            for key,rate,raw,_,_ in read(sfz,asset):
                f,b=map(int,pick['choices'][str(key)].split(':'))
                assert b==8 # current actual selected data; decoder not generalized
                a=raw.astype('f8')/32768
                a=a if f==1 else resample_poly(a,1,f,window=('kaiser',9.0),padtype='line')
                q=np.clip(np.rint(a*128),-128,127).astype('i4')
                meta+=32 # hypothetical index region descriptor, not final format
                for i in range(0,len(q),block):
                    bb=encode_payload(q[i:i+block],b)
                    z=zlib.compress(bb,9)
                    assert zlib.decompress(z)==bb
                    payload+=len(z); entries+=1
                meta+=4 # terminal offset for this zone
            meta+=entries*4
            rows[block]={'blob_B':payload,'index_and_descriptors_B':meta,'combined_B':payload+meta,
                'all_chosen_quantized_words_roundtrip':True,'cache_six_owners_two_blocks_if_fully_decoded_B':block*12}
        cases[str(lev)]=dict(no_restart_payload_B=pick['payload_B'],restart_profiles=rows)
    return dict(checked=cases, not_a_sector_allocation=True, no_existing_breaks_or_decoders_included=True)
if __name__=='__main__':
    print(json.dumps(analyze(*map(Path,sys.argv[1:])),indent=2))
