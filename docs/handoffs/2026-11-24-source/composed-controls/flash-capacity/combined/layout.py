"""Run lossless audio verification and build a virtual sector-ownership plan.

No writes to a device, no firmware image, no snapshot of user flash. Main
reports an offline test layout, not permission to reclaim old VM sectors.
"""
import argparse
import hashlib
import importlib.util
import json
import re
from pathlib import Path

S=4096
K=1024
APP_HIGH=0x8b000
BLE_HIGH=0x9b000
SLOT2=0xd6000
SAMPLE_TOP=0xf4000
D9_OTHER=6159    # actual diagnostic non-sample payload, stays reserved
PACK_ALIGNED=176*K
COW_BANK=19769   # measured append delta in current dense whole-bank replacement trial

def align(n):return (n+S-1)//S*S
def need(x,msg):
    if not x:raise ValueError(msg)
def import_helper(f):
    s=importlib.util.spec_from_file_location('stream_storage',f)
    m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
    return m

def sample_header(h):
    fields=re.findall(r'uint8_t (x0x_smp_(?:crash|ride|hh))\[(\d+)\] = \{([^}]+)\}',h.read_text())
    expected={'x0x_smp_crash':21650,'x0x_smp_ride':23500,'x0x_smp_hh':10125}
    result={}
    for name,n,body in fields:
        v=bytes(int(i.strip(),0) for i in body.split(',') if i.strip())
        need(len(v)==int(n)==expected[name],name)
        result[name]=v
    need(set(result)==set(expected),'909 input set')
    return result

def break_bytes(path):
    out={1:[],2:[]}
    for x in json.loads((path/'default-breaks.json').read_text())['assets']:
        b=(path/(x['id']+'.adpcm')).read_bytes()
        need(len(b)==x['adpcm_bytes'] and hashlib.sha256(b).hexdigest()==x['adpcm_sha256'],'BREAK input hash or extent')
        out[x['slot']].append(b)
    need([len(out[i]) for i in [1,2]]==[3,2],'vanilla corpus')
    return out

def validate(used):
    prev=APP_HIGH
    for start,stop,name in sorted(used):
        need(start>=prev and start%S==stop%S==0 and start<stop<=SAMPLE_TOP,f'overlap/bad extent {name}')
        prev=stop

def test_plan(journal_kib,d9_bytes,br2_bytes):
    need(journal_kib in (0,8,16),'journal size must be qualified separately')
    fdata_lo=SAMPLE_TOP-align(d9_bytes)
    directory=fdata_lo-2*S
    br2_end=SLOT2+align(br2_bytes)
    need(br2_end<=directory,'existing BREAK collides with directory')
    used=[(APP_HIGH,BLE_HIGH,'leave BLE untouched'),
          (SLOT2,br2_end,'preserved factory BREAK slot 2'),
          (directory,fdata_lo,'two independent directories'),
          (fdata_lo,SAMPLE_TOP,'exact reconstructed 909 and fixed tables')]
    free=[(br2_end,directory,'slot 2 sectors')] if br2_end<directory else []
    if journal_kib:
        free.append((0xa5000,0xa7000,'old PANEL pair, migration required'))
    if journal_kib==16:
        free.append((0x9d000,0x9f000,'old SETTINGS pair, migration required'))
    validate(used+free)
    data=sum(stop-start for start,stop,_ in free)
    return dict(journal_kib_conditional=journal_kib,fdata_range=[hex(fdata_lo),hex(SAMPLE_TOP)],
        break2_range=[hex(SLOT2),hex(br2_end)],directories=[hex(directory),hex(fdata_lo)],
        virtual_free_extents=[[hex(a),hex(b),n] for a,b,n in free],
        data_before_reserve_B=data,overflow_full_pack_before_reserve_B=PACK_ALIGNED-data,
        max_payload_with_tested_whole_bank_replace_reserve_B=max(0,data-align(COW_BANK)))

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--stream-helper',type=Path,required=True)
    p.add_argument('--909-header',dest='drum_header',type=Path,required=True)
    p.add_argument('--break-assets',type=Path,required=True)
    a=p.parse_args();codec=import_helper(a.stream_helper)
    d9=sample_header(a.drum_header)
    d9_packed={n:codec.huff_asset(b,256,256,False) for n,b in d9.items()}
    fdata=D9_OTHER+sum(d9_packed.values())
    br=break_bytes(a.break_assets)
    brp={z:512+sum(min(codec.huff_asset(b,256,16,False),codec.huff_asset(b,256,256,False))for b in clips)for z,clips in br.items()}
    need(fdata==57120 and brp=={1:55239,2:46037},'corpus or packing differs from measurements')
    rows=[test_plan(x,fdata,brp[2])for x in (0,8,16)]
    need([r['data_before_reserve_B'] for r in rows]==[8*K,16*K,24*K],'combined ledger')
    need([r['max_payload_with_tested_whole_bank_replace_reserve_B'] for r in rows]==[0,0,4*K],'reserve charged exactly once')
    print(json.dumps(dict(offline=True,verified_909_adpcm_B=sum(map(len,d9.values())),
        verified_five_break_adpcm_B=sum(map(len,br[1]+br[2])),encoded_909_payloads=d9_packed,
        fdata_B=fdata,encoded_break_slot_B=brp,cow_bank_reserve_B=align(COW_BANK),
        options=rows),indent=2))
if __name__=='__main__':main()
