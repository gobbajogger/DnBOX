"""Fail-closed offline oracle. Never a native timing gate or runner override."""
import json,sys
from pathlib import Path

def compare(a,b,assert_same_elf=False):
    fail=[]
    for x in [a,b]:
        n=x['run']
        if x['prezero_settled_good']<4:
            fail.append(f'{n}: fewer than four exact pre-scene settled owner replies')
        if x['first_consumed_dsp_event_frame'] is None:
            fail.append(f'{n}: no effective producer frame for first scene event')
        if x.get('first_late_dsp_half_index') is None and x.get('observed_no_late') is not True:
            fail.append(f'{n}: first late half—not reported by runner')
    if a['scene_sha256']!=b['scene_sha256'] or a['events']!=b['events']:
        fail.append('scene tape differs')
    if a['first_consumed_dsp_event_frame']!=b['first_consumed_dsp_event_frame']:
        fail.append('effective renderer event frame differs')
    if a.get('complete_admission_trace_sha256') is None or b.get('complete_admission_trace_sha256') is None:
        fail.append('complete renderer-consumed event-frame receipt missing')
    elif a['complete_admission_trace_sha256']!=b['complete_admission_trace_sha256']:
        fail.append('complete renderer-consumed event-frame receipt differs')
    if assert_same_elf and a['pcm_sha256']!=b['pcm_sha256']:
        fail.append('exact-ELF repeat PCM differs')
    return {'compared':[a['run'],b['run']], 'eligible_for_comparison':not fail, 'failures':fail,
            'same_pcm':a['pcm_sha256']==b['pcm_sha256'],
            'same_first_dac_nonzero_frame':a['first_pcm_nonzero_frame']==b['first_pcm_nonzero_frame']}
if __name__=='__main__':
    rows=list(map(json.loads,Path(sys.argv[1]).read_text().splitlines()))
    res=[compare(rows[4],rows[5],True),compare(rows[0],rows[2]),compare(rows[1],rows[3])]
    print(json.dumps(res,indent=2))
    sys.exit(0 if all(r['eligible_for_comparison'] for r in res) else 2)
