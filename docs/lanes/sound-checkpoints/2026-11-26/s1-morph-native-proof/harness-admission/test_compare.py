from compare import compare

def fixture(run):
    return dict(run=run, prezero_settled_good=4, first_consumed_dsp_event_frame=256,
                observed_no_late=True, first_late_dsp_half_index=None,
                scene_sha256='unchanged', pcm_sha256='same', events=922,
                complete_admission_trace_sha256='complete-present',first_pcm_nonzero_frame=768)
a,b=fixture('a'),fixture('b')
assert compare(a,b,True)['eligible_for_comparison']
for change in (dict(prezero_settled_good=3),dict(first_consumed_dsp_event_frame=None),
               dict(first_consumed_dsp_event_frame=257),dict(observed_no_late=False),
               dict(complete_admission_trace_sha256='different'),dict(events=921),dict(pcm_sha256='new')):
    assert not compare(a,b|change,True)['eligible_for_comparison'],change
print('PASS offline gate contract on modeled positive + seven negative receipts')
