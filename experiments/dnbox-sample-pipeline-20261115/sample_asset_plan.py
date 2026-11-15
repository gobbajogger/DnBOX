# SPDX-License-Identifier: GPL-3.0-only
"""Pure private sample-asset composition and append-only install preflight.

No flash/installer I/O. No app-code placement. Existing compiled banks and all
existing sample images stay unchanged. The caller authenticates owner permission,
the source catalog digest, the current device layout and complete current bytes.
A returned plan is not permission or proof to write hardware.
"""
import hashlib,json,struct,zlib
from collections import Counter
HEADER=512
MAGIC=0x504d5346
def sha(b):return hashlib.sha256(b).hexdigest()
def catalog_digest(catalog):
    return sha(json.dumps([{'id':r['id'],'group':r['group'],'zone_sha256':sha(bytes(r['zone'])),'data_sha256':sha(bytes(r['data']))} for r in catalog],sort_keys=True,separators=(',',':')).encode())
def _zone(zone,data):
    if not isinstance(zone,(bytes,bytearray)) or len(zone)!=28:raise ValueError('exact 28-byte source zone required')
    if not isinstance(data,(bytes,bytearray)):raise ValueError('exact encoded bytes required')
    off,n,ls,le,rate,root,pred,idx,lo,hi,looped=struct.unpack('<5Ihh4B',zone)
    if not n or len(data)!=(n+1)//2 or not 0<rate<=4<<16 or idx>88 or lo>hi or looped not in(0,1) or ls>=n or le>=n or ls>le:
        raise ValueError('invalid source zone metadata or span')
    # Original off belongs to another image. It is not part of an asset identity.
    canonical=struct.pack('<I',0)+bytes(zone[4:])
    return canonical,bytes(data)
def _fingerprint(zone,data):return sha(_zone(zone,data)[0]+bytes(data))
def validate_catalog(catalog,expected_digest):
    if not isinstance(catalog,list) or not catalog or len(catalog)>48:raise ValueError('complete authenticated source catalog required')
    ids=set();groups=[];group_counts=Counter()
    for r in catalog:
        if not isinstance(r,dict) or not isinstance(r.get('id'),str) or not r['id'] or r['id'] in ids:raise ValueError('unique asset IDs required')
        ids.add(r['id'])
        name=r.get('group')
        if not isinstance(name,str) or not 1<=len(name)<=8 or any(ord(c)<32 or ord(c)>126 for c in name):raise ValueError('ASCII source group required')
        if not groups or groups[-1]!=name:
            if name in groups:raise ValueError('noncanonical repeated source group')
            groups.append(name)
        group_counts[name]+=1
        if group_counts[name]>16 or len(groups)>3:raise ValueError('source group count exceeds native layout')
        _zone(r.get('zone'),r.get('data'))
    if not isinstance(expected_digest,str) or len(expected_digest)!=64 or catalog_digest(catalog)!=expected_digest:raise ValueError('catalog identity changed')
def parse_image(image,capacity):
    if type(capacity)is not int or capacity<HEADER:raise ValueError('invalid authenticated slot capacity')
    if image is None:return []
    if not isinstance(image,(bytes,bytearray)):raise ValueError('unknown existing slot image')
    image=bytes(image)
    if len(image)<HEADER or len(image)>capacity:raise ValueError('existing slot span')
    magic,ver,nz,rsv,name,dlen,crc=struct.unpack_from('<IHBB8sII',image)
    if magic!=MAGIC or ver!=1 or not 1<=nz<=16 or len(image)!=HEADER+dlen or zlib.crc32(image[HEADER:])!=crc:raise ValueError('existing slot invalid; never overwrite unknown data')
    out=[]
    for i in range(nz):
        zone=image[32+28*i:60+28*i];off,n=struct.unpack_from('<II',zone)
        length=(n+1)//2
        if off>dlen or length>dlen-off:raise ValueError('existing zone span')
        data=image[HEADER+off:HEADER+off+length];_zone(zone,data);out.append((zone,data))
    return out
def compose_group(records):
    if not records or len(records)>16:raise ValueError('1..16 zones per original group')
    name=records[0]['group']
    if any(r['group']!=name for r in records):raise ValueError('cannot merge different source group namespaces')
    data=bytearray();zones=[]
    for r in records:
        zone,audio=_zone(r['zone'],r['data'])
        zones.append(struct.pack('<I',len(data))+zone[4:]);data.extend(audio)
    h=bytearray(HEADER)
    struct.pack_into('<IHBB8sII',h,0,MAGIC,1,len(zones),0,name.encode('ascii').ljust(8,b'\0'),len(data),zlib.crc32(data))
    for i,z in enumerate(zones):h[32+28*i:60+28*i]=z
    return bytes(h)+bytes(data)
def plan_install(catalog,expected_digest,current_images,capacities,selected_ids=None):
    """Default all; [] is no-op. Append original groups ONLY after occupied slots.

    Already-present exact zone+data occurrences are retained in place. Occupied,
    unknown or corrupt slots are NEVER rewritten, nor are holes before later
    samples filled: this preserves every existing user-loop ordinal. Failure
    produces no partial writes. Readback/snapshot guards and power-failure-safe
    transport are mandatory downstream, not supplied by this pure planner.
    """
    if not isinstance(catalog,list):raise ValueError('complete source catalog required')
    frozen=[]
    for r in catalog:
        if not isinstance(r,dict):raise ValueError('source catalog record required')
        c=dict(r)
        for key in ('zone','data'):
            if not isinstance(c.get(key),(bytes,bytearray)):raise ValueError('exact source bytes required')
            c[key]=bytes(c[key])
        frozen.append(c)
    catalog=frozen
    validate_catalog(catalog,expected_digest)
    if not isinstance(current_images,(list,tuple)) or not isinstance(capacities,(list,tuple)) or len(current_images)!=len(capacities) or not 1<=len(capacities)<=3:raise ValueError('complete authenticated slot layout required')
    current_images=tuple(bytes(b)if isinstance(b,(bytes,bytearray))else b for b in current_images)
    capacities=tuple(capacities)
    current=[parse_image(b,c)for b,c in zip(current_images,capacities)]
    known=[r['id']for r in catalog]
    if selected_ids is None:chosen=set(known)
    else:
        if not isinstance(selected_ids,(list,tuple)) or any(not isinstance(i,str)for i in selected_ids) or len(set(selected_ids))!=len(selected_ids) or any(i not in known for i in selected_ids):raise ValueError('unknown or duplicate selected identity')
        chosen=set(selected_ids)
    selected=[r for r in catalog if r['id']in chosen]
    available=Counter(_fingerprint(z,d)for slot in current for z,d in slot)
    missing=[]
    for r in selected:
        f=_fingerprint(r['zone'],r['data'])
        if available[f]:available[f]-=1
        else:missing.append(r)
    groups=[]
    for r in missing:
        if not groups or groups[-1][0]['group']!=r['group']:groups.append([])
        groups[-1].append(r)
    last=max((i for i,z in enumerate(current)if z),default=-1)
    writes=[];next_slot=last+1
    for group in groups:
        image=compose_group(group)
        while next_slot<len(capacities) and len(image)>capacities[next_slot]:next_slot+=1
        if next_slot==len(capacities):raise ValueError('no safe append capacity; existing samples and ordinals preserved')
        writes.append({'slot_index':next_slot,'image':image,'asset_ids':[r['id']for r in group]})
        next_slot+=1
    before=[_fingerprint(z,d)for slot in current for z,d in slot]
    proposed=list(current_images)
    for w in writes:
        assert proposed[w['slot_index']]is None
        proposed[w['slot_index']]=w['image']
    after=[_fingerprint(z,d)for b,c in zip(proposed,capacities)for z,d in parse_image(b,c)]
    if after[:len(before)]!=before:raise AssertionError('existing menu ordinals changed')
    return {'selected_ids':[r['id']for r in selected],'writes':writes,'existing_images_sha256':[None if b is None else sha(bytes(b))for b in current_images],'existing_menu_fingerprints':before,'proposed_images':proposed,'capacity_bytes':list(capacities),'device_write_qualified':False}

