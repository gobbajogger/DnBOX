"""Submit a data-only job to a running DnBOX AUv3 Bench. Does not launch it."""
import argparse
import json
import os
from pathlib import Path
import re
import tempfile

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument("id")
parser.add_argument("profile",choices=("master","fanin","static","physical","dynamic","jupiter-gain","jupiter-response"))
parser.add_argument("--root",type=Path,default=Path.home()/"DnBOX-AU-Bench")
parser.add_argument("--step",action="append",default=[],metavar="FRAME:VALUE")
args=parser.parse_args()
if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_-]{0,47}",args.id):
    parser.error("job ID may only contain letters, digits, underscores and hyphens")
recipe={"id":args.id,"profile":args.profile}
if args.step:
    try:
        pairs=[s.split(":") for s in args.step]
        steps=[{"frame":int(a),"value":float(b)} for a,b in pairs]
        frames=[s["frame"] for s in steps]
        if (args.profile!="dynamic" or len(steps)>8 or not 16384<=frames[0]<=frames[-1]<82534
            or frames!=sorted(set(frames)) or not all(0<=s["value"]<=1 for s in steps)
            or steps[-1]["value"]!=0.3345831036567688):
            raise ValueError
    except ValueError:
        parser.error("dynamic requires ordered steps within the cue, in [0,1], ending at 0.3345831036567688")
    recipe["cutoffSteps"]=steps
pending=args.root/"requests"
queue=pending/f"{args.id}.json"
if (args.root/"jobs"/args.id).exists():
    parser.error("job ID already used")
fd,tmp=tempfile.mkstemp(dir=pending,prefix=".job-")
try:
    with os.fdopen(fd,"w") as f:
        json.dump(recipe,f,allow_nan=False)
        f.write("\n")
    os.link(tmp,queue)
finally:
    os.unlink(tmp)
print(args.root/"jobs"/args.id/"result.json")
