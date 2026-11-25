from pathlib import Path
import sys,json,numpy as np
from measure import margin, db, rms, floats, SR
root=Path(sys.argv[1]); capture=root/'matrix.json'
if capture.exists():
 for row in json.loads(capture.read_text())['cases']:
  v={k:floats(root/p) for k,p in row['captures'].items()}
  a,b=v['original-wet'],v['compact-wet'];da,dd=v['original-dry'],v['compact-dry']
  fx_a,fx_b=a-da,b-dd
  x=margin(a,b)
  dry=round(float(db(rms(da-dd),rms(da))),3)
  eq=(np.array_equal(a,da),np.array_equal(b,dd))
  if eq==(False,False):
   fx=round(float(db(rms(fx_a-fx_b),rms(fx_a))),3)
   fxlast=round(float(db(rms(fx_b[-SR:]),rms(fx_a[-SR:]))),3)
  else:fx=fxlast=None
  print(row['name'],json.dumps(dict(**x,dry_null_rel_db=dry,fx_null_rel_db=fx,last_1s_fx_level_error_db=fxlast,exact_bypass=eq)))
