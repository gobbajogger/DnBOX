"""Summarize four exact arm receipts; keep synthetic audio and NOR out of Git."""
import hashlib,json
from pathlib import Path
import sys
h=Path(sys.argv[1])
arms=('owner-epoch-link-wet','delay-center-exact-wet')
result={}
for model in ('elite','pro'):
    rows=[]
    for arm in arms:
        q=h/f'{arm}-{model}'
        load=lambda ext:json.loads(q.with_suffix(ext).read_text())
        state=load('.witness.json')
        timing=load('.timing.json')
        controls=load('.controls.json')
        r=Path(q.with_suffix('.output').read_text().strip())/'wet'
        pcm=(r/'audio.s16le').read_bytes()
        assert 'skipped 0 cycles' in q.with_suffix('.emulator.log').read_text()
        assert 'flash: 0 erases / programs' in q.with_suffix('.emulator.log').read_text()
        p=Path((h/f'{arm}.projection').read_text().strip())
        a=(p/f'{model}-authored-nor.bin').read_bytes()
        assert a==(r/'disposable.bin').read_bytes()
        rows.append(dict(arm=arm,timing=timing,ring=state,owner=controls['owner'],
                         control_readback=controls['startEndIndependentReadback'],
                         notes=controls['active'],virtual_nor_unchanged=True,
                         pcm_sha256=hashlib.sha256(pcm).hexdigest()))
    r0,r1=rows
    assert r0['notes']==r1['notes'] and r0['control_readback']==r1['control_readback']
    for k in ('wet','fb','enabled','filled','stride','settled_tap_audio_frames'):assert r0['ring'][k]==r1['ring'][k]
    assert r0['timing']['frames']==r1['timing']['frames']
    assert r0['pcm_sha256']!=r1['pcm_sha256'], 'Trial did not alter the armed delay output'
    result[model]=rows
print(json.dumps(result,indent=2))
