from pathlib import Path
from hashlib import sha256
import json,sys
def load(name):
    p=Path(name)
    header,*rows=map(json.loads,p.read_text().splitlines())
    by={r['id']:r for r in rows}
    assert header['count']==len(rows)==len(by)==906
    assert sorted(by)==list(range(906))
    return header,by,sha256(p.read_bytes()).hexdigest()
a,x,sha0=load(sys.argv[1]);b,y,sha1=load(sys.argv[2])
for k in ('same_note_tape','tuning_rows','identity_sha256','mode'):
    assert a[k]==b[k]
for i in x:
    for k in ('label','ref_sha256','trial_sha256','same_note_events','authored_tuning'):
        assert x[i][k]==y[i][k],(i,k)
print(json.dumps({'cases':906,'same_reference_pcm':906,'same_candidate_pcm':906,
                  'scope':'fixed-note-tape host audio, not native timing or fit',
                  'old_full_receipt_sha256':sha0,'new_full_receipt_sha256':sha1,
                  'old_inputs':a['inputs'],'new_inputs':b['inputs']},indent=2))
