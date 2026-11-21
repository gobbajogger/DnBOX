"""Join original 694 case identities to the verified frontend delay targets."""
import argparse
import csv
import json
import struct
from collections import Counter
from pathlib import Path
from statistics import median

def f32(x):
    return struct.unpack("<f",struct.pack("<f",float(x)))[0]

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("commands",type=Path)
    ap.add_argument("targets",type=Path)
    ap.add_argument("out",type=Path)
    args=ap.parse_args()
    rows=[]
    cases=json.loads(args.commands.read_text())["cases"]
    with args.targets.open() as f:
        for i,(case,row) in enumerate(zip(cases,csv.DictReader(f),strict=True)):
            assert int(row["index"])==i
            raw=dict(case["commands"])
            assert len(case["commands"])==144
            for key,param in (("on",41),("feedback",42),("mix",44)):
                assert f32(row[key])==f32(raw[param])
            assert f32(row["live_min_seconds"])==f32(.00036281178472563624)
            assert f32(row["live_max_seconds"])==2.5
            resolved=f32(row["seconds"])
            wet=f32(row["on"])*f32(row["mix"])>0
            rows.append(dict(index=i,bank=case["bank"],preset_index=case["preset_index"],
                 name=case["name"],uid=case["uid"],raw_time_sec=f32(raw[43]),
                 restored_time_sec=resolved,on=int(float(row["on"])),feedback=f32(row["feedback"]),
                 mix=f32(row["mix"]),tempo_bpm=f32(row["bpm"]),tempo_sync=int(float(row["sync"])),
                 wet_at_restored_target=int(wet),live_min_sec=f32(row["live_min_seconds"]),
                 live_max_sec=f32(row["live_max_seconds"])))
    assert len(rows)==694
    with (args.out/"programs.csv").open("w",newline="") as f:
        w=csv.DictWriter(f,fieldnames=rows[0],lineterminator="\n");w.writeheader();w.writerows(rows)
    wet=[r for r in rows if r["wet_at_restored_target"]]
    dry=[r for r in rows if not r["wet_at_restored_target"]]
    def counts(rr):
        return dict(cases=len(rr),on=sum(r["on"] for r in rr),
                    mix_positive=sum(r["mix"]>0 for r in rr),feedback_zero=sum(r["feedback"]==0 for r in rr),
                    tempo_sync=sum(r["tempo_sync"] for r in rr),
                    feedback_min=min(r["feedback"] for r in rr),feedback_max=max(r["feedback"] for r in rr),
                    distinct_resolved_times=len({r["restored_time_sec"] for r in rr}),
                    raw_vs_restored_time_changed=sum(r["raw_time_sec"]!=r["restored_time_sec"] for r in rr),
                    raw_min_sec=min(r["raw_time_sec"] for r in rr),raw_max_sec=max(r["raw_time_sec"] for r in rr),
                    restored_min_sec=min(r["restored_time_sec"] for r in rr),
                    restored_median_sec=median(r["restored_time_sec"] for r in rr),
                    restored_max_sec=max(r["restored_time_sec"] for r in rr))
    summary=dict(all=counts(rows),wet=counts(wet),dry=counts(dry),banks=dict(Counter(r["bank"] for r in rows)),
                 targets_with_full_time_sweep=len(rows))
    (args.out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")

if __name__=="__main__":
    main()
