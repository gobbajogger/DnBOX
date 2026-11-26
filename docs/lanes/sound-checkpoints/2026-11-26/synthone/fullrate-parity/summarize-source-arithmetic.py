#!/usr/bin/env python3
"""Compare complete host stereo runs by stable row, reject missing or changed reference data."""
import json, math, sys
from pathlib import Path
def read(name):
    header,*rows=map(json.loads,Path(name).read_text().splitlines())
    byid={r["id"]:r for r in rows}
    assert len(rows)==len(byid)==header["count"]==906
    assert set(byid)==set(range(906))
    return header,byid
def whole(row,key):
    result=row["metrics"]["whole"][key]
    assert len(result)==2 and all(math.isfinite(v) for v in result)
    return result
def summarize(rows):
    worst=max(rows,key=lambda r:max(whole(r,"residual_dbr")))
    return dict(count=len(rows),
        both_waveform_at_most_dbr={str(t):sum(max(whole(r,"residual_dbr"))<=t for r in rows) for t in (-20,-40,-60,-80)},
        max_abs_level_db=max(max(map(abs,whole(r,"level_db"))) for r in rows),
        bit_identical=sum(r["ref_sha256"]==r["trial_sha256"] for r in rows),
        worst=dict(id=worst["id"],label=worst["label"],residual_dbr=whole(worst,"residual_dbr")))
if __name__=="__main__":
    old_header,old=read(sys.argv[1])
    new_header,new=read(sys.argv[2])
    for key in ("identity_sha256","same_note_tape","tuning_rows","mode"):
        assert old_header[key]==new_header[key]
    for i,row in new.items():
        for key in ("ref_sha256","same_note_events","label","authored_tuning"):
            assert row[key]==old[i][key],(i,key)
    outliers=[r for i,r in new.items() if max(whole(old[i],"residual_dbr"))>-40]
    result=dict(old=summarize(list(old.values())),new=summarize(list(new.values())),
        prior_outliers=dict(repaired=sum(max(whole(r,"residual_dbr"))<=-40 for r in outliers),
          rows=[dict(id=r["id"],before=whole(old[r["id"]],"residual_dbr"),after=whole(r,"residual_dbr")) for r in outliers]),
        new_outliers=[i for i,r in new.items() if max(whole(r,"residual_dbr"))>-40 and max(whole(old[i],"residual_dbr"))<=-40],
        same_reference_pcm=906, old_run=old_header, new_run=new_header)
    print(json.dumps(result,indent=2,ensure_ascii=False))
