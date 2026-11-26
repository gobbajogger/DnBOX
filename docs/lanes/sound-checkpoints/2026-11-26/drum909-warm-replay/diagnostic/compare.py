import json, sys
old,new=map(lambda n:json.load(open(n)),sys.argv[1:]); out=[]
assert old['unchanged_tape_events']==new['unchanged_tape_events']==922
for a,b in zip(old['result'],new['result'],strict=True):
    assert a['model']==b['model'] and a['tape']==b['tape'] and a['project_input_records']==b['project_input_records']
    am,bm=a['raw_result'],b['raw_result']
    assert b['unchanged_disposable_virtual_nor'] and bm['skipped_cycles']==0 and bm['core1']['bad_mailbox']==0
    assert bm['frames']>0 and bm['peak']>0 and bm['settled_owner_readbacks']>=4
    out.append(dict(model=a['model'],old_us=am['worst_us'],new_us=bm['worst_us'],
                    old_late=am['raw_late'],new_late=bm['raw_late'],old_xruns=am['xruns'],new_xruns=bm['xruns'],
                    old_halves=am['halves'],new_halves=bm['halves'],same_byte_exact_pcm=a['pcm']==b['pcm'],
                    old_first_valid_owner_ms=a['first_valid_owner_ms'],new_first_valid_owner_ms=b['first_valid_owner_ms'],
                    pre_tape_healthy_count=b['prior_to_tape_owner'],legacy_timeline_old=a['legacy_hit_timeline_words'],legacy_timeline_new=b['legacy_hit_timeline_words']))
print(json.dumps(dict(records_identical=True,original_tape_bytes_identical=True,scope='diagnostic; external bytes identical, actual producer admission unproven; mixed workloads, PCM or elapsed differences cannot be causally attributed',results=out),indent=2))
