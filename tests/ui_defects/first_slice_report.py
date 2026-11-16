# SPDX-License-Identifier: GPL-3.0-only
"""Normalize partial firmware evidence without inventing acceptance."""
import argparse
import copy
import hashlib
import json
from pathlib import Path

def normalize(raw, artifacts, negatives):
    source=raw["sourceClosureSha256"]
    checks=[]
    for cid in range(1,25):
        c=raw.get("checks",{}).get(str(cid),{})
        checks.append({"id":cid,"status":c.get("status","NOT_RUN"),
            "evidence":copy.deepcopy(artifacts) if c.get("status") in {"PASS","FAIL"} else [],
            "failureCount":len(c.get("failures",[])),"limits":[c.get("evidence","Outside implemented slice.")]})
    screens=[]
    for s in raw.get("screens",[]):
        defects=["Human full-size review/scoring pending."]
        defects.extend(f"Check {cid}: {len(c.get('failures',[]))} measured findings" for cid,c in s.get("checks",{}).items() if c.get("status")=="FAIL")
        screens.append({**s.get("png",{}),"id":s["id"],"score":None,"defects":defects,
                        "applicability":"APPLICABLE","sourceClosureSha256":source})
    return {"schema":"dnbox-firmware-ui-defects/1","sourceClosureSha256":source,
        "checks":checks,"screens":screens,"negatives":copy.deepcopy(negatives),
        "humanDefects":[{"id":n["defectId"]} for n in negatives],"coverageComplete":False,
        "run":{"exit":raw.get("detectorExit",1),"firstAttempt":False,"sourceClosureSha256":source,
               "limits":"Partial development detector evaluation; formal exact-pushed-source qualification NOT_RUN."},
        "rendererRun":copy.deepcopy(raw.get("run",{})),"coverage":copy.deepcopy(raw.get("coverage",{})),
        "limitations":copy.deepcopy(raw.get("limitations",[]))+["Partial report conversion never supplies human acceptance or formal qualification."]}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--report",type=Path,required=True);p.add_argument("--negatives",type=Path,required=True)
    p.add_argument("--output",type=Path,required=True);a=p.parse_args()
    raw=json.loads(a.report.read_text());source=raw["sourceClosureSha256"]
    a.output.mkdir(parents=True,exist_ok=False);(a.output/"artifacts").mkdir()
    def stage(item):
        path=Path(item["path"]);data=path.read_bytes();digest=hashlib.sha256(data).hexdigest()
        if digest!=item["sha256"]:raise ValueError(f"Artifact bytes changed: {path.name}")
        rel="artifacts/"+digest[:12]+"-"+path.name;(a.output/rel).write_bytes(data)
        return {**item,"path":rel}
    artifacts=[stage(e) for e in raw.get("evidenceArtifacts",[])]
    report_bytes=a.report.read_bytes()
    artifacts.append(stage({"path":str(a.report),"sha256":hashlib.sha256(report_bytes).hexdigest(),
                           "sourceClosureSha256":source,"kind":"actual-render-detector-report"}))
    for s in raw.get("screens",[]):s["png"]=stage(s["png"])
    negatives=json.loads(a.negatives.read_text())
    for n in negatives:
        evidence=[]
        for label in ("old","current"):
            path=a.negatives.parent/(n["defectId"]+"-"+label+".json");data=path.read_bytes();case=json.loads(data)
            if case.get("sourceClosureSha256")!=n[label+"SourceClosureSha256"] or case.get("exit")!=n[label+"Exit"]:
                raise ValueError("Negative-control case identity/exit differs")
            # The paired receipt authenticates both old/current identities in its
            # own bytes; current report scope is not a claim old source==current.
            evidence.append(stage({"path":str(path),"sha256":hashlib.sha256(data).hexdigest(),
                                   "sourceClosureSha256":source,"kind":"paired-old-current-case-receipt"}))
        n["evidence"]=evidence
    result=normalize(raw,artifacts,negatives)
    (a.output/"report.json").write_text(json.dumps(result,indent=2)+"\n")
    print("PARTIAL_REPORT_WRITTEN: product gate remains blocked")
    return 0

if __name__=="__main__":raise SystemExit(main())
