# SPDX-License-Identifier: GPL-3.0-only
"""Pure exact-bank selection/header helpers. No installer, filesystem or device writes.

The caller authenticates the manifest/owner authority and supplies the reviewed
break_pack module from the admitted source closure. Digest equality is content
identity, not proof of permission. None means no embedded bank in a separately
verified empty-bank base, NOT removal of existing user samples or compiled loops.
No capacity, native fit, playback, installation or publication success is implied.
"""
import hashlib
import re
import break_pack

def select_pack(bank, expected_bank_sha256, assets, selected_ids=None):
    """Validate the complete canonical bank and every manifest asset first.

    Default is all. An explicit empty list returns None. Subsets keep manifest
    order, original encoded bytes, frame counts, bars and sample rate. Unknown
    or duplicate IDs, malformed metadata and any changed byte fail before output.
    Source bank and manifest are never mutated. This does not validate OS mapping
    or authenticate a caller-supplied manifest; those are upstream obligations.
    """
    if not isinstance(expected_bank_sha256,str) or not re.fullmatch('[0-9a-f]{64}',expected_bank_sha256):
        raise ValueError('expected immutable bank digest required')
    data=bytes(bank)
    if hashlib.sha256(data).hexdigest()!=expected_bank_sha256:
        raise ValueError('bank identity changed')
    parsed=break_pack.parse_pack(data)
    if parsed is None:raise ValueError('invalid bank')
    rate,loops=parsed
    if not isinstance(assets,list) or len(assets)!=len(loops):raise ValueError('complete ordered asset manifest required')
    ids=[]
    for spec,loop in zip(assets,loops):
        if not isinstance(spec,dict):raise ValueError('invalid asset')
        identity=spec.get('id')
        if not isinstance(identity,str) or not identity or identity in ids:raise ValueError('invalid or duplicate asset identity')
        ids.append(identity)
        if (spec.get('name')!=loop['name'] or type(spec.get('frames')) is not int or spec.get('frames')!=loop['nsamples'] or
            type(spec.get('sample_rate')) is not int or spec.get('sample_rate')!=rate or
            type(spec.get('bars',loop['bars'])) is not int or spec.get('bars',loop['bars'])!=loop['bars'] or
            type(spec.get('channels',1)) is not int or spec.get('channels',1)!=1 or spec.get('codec','IMA_ADPCM4_LOW_NIBBLE_FIRST')!='IMA_ADPCM4_LOW_NIBBLE_FIRST' or
            type(spec.get('initial_predictor',0)) is not int or spec.get('initial_predictor',0)!=0 or
            type(spec.get('initial_index',0)) is not int or spec.get('initial_index',0)!=0 or
            type(spec.get('adpcm_bytes',len(loop['adpcm']))) is not int or spec.get('adpcm_bytes',len(loop['adpcm']))!=len(loop['adpcm']) or
            spec.get('adpcm_sha256')!=hashlib.sha256(loop['adpcm']).hexdigest()):
            raise ValueError('asset metadata or encoded identity changed: '+identity)
    if break_pack.encode_pack(loops,rate)!=data:raise ValueError('noncanonical bank bytes')
    if selected_ids is None:chosen=set(ids)
    else:
        if not isinstance(selected_ids,(list,tuple)) or any(not isinstance(v,str)for v in selected_ids):raise ValueError('explicit ID list required')
        if len(set(selected_ids))!=len(selected_ids) or any(v not in ids for v in selected_ids):raise ValueError('unknown or duplicate selected identity')
        chosen=set(selected_ids)
    selected=[v for identity,v in zip(ids,loops)if identity in chosen]
    return break_pack.encode_pack(selected,rate)if selected else None

def header_for_pack(pack):
    """Emit source-compatible immutable bank declarations with ACTUAL pack rate.

    Unlike the old write_export_header helper, this never substitutes 44.1kHz
    for a 22.05kHz bank. Header generation is not source/build integration.
    None emits no audio arrays; callers must use a verified empty-bank base.
    """
    if pack is None:rate,loops=44100,[]
    else:
        parsed=break_pack.parse_pack(pack)
        if parsed is None:raise ValueError('invalid bank')
        rate,loops=parsed
    lines=['/* Generated exact selected bank. No sample provenance or public redistribution grant implied. */','#pragma once','#include <stdint.h>',
           '#define X0X_NBREAKS '+str(len(loops)), '#define X0X_BREAK_RATE '+str(rate)+'u']
    for i,loop in enumerate(loops):
        if any(ord(ch)<32 or ord(ch)>126 for ch in loop['name']):raise ValueError('nonprintable bank name')
        data=loop['adpcm'];lines.append('static const uint8_t X0X_BREAK_'+str(i)+'['+str(len(data))+'] = {')
        lines.extend('    '+','.join(str(v)for v in data[k:k+24])+','for k in range(0,len(data),24));lines.append('};')
    lines.append('typedef struct { const char *name; const uint8_t *adpcm; uint32_t n; uint8_t bars; } x0x_break_t;')
    if loops:
        lines.append('static const x0x_break_t X0X_BREAKS[X0X_NBREAKS] = {')
        for i,loop in enumerate(loops):
            name=loop['name'].replace('\\','\\\\').replace('"','\\"').replace('?', '\\?')
            lines.append('    {"'+name+'", X0X_BREAK_'+str(i)+', '+str(loop['nsamples'])+'u, '+str(loop['bars'])+'},')
        lines.append('};')
    else:lines.append('static const x0x_break_t X0X_BREAKS[1] = {{0,0,0,0}};')
    return '\n'.join(lines)+'\n'
