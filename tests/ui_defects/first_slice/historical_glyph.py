#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Explicit composed fixture: authentic f254 glyph body in frozen current host.
This is NOT a recovery or qualification of the complete historical firmware.
"""
import argparse,hashlib,json,shutil
from pathlib import Path
from instrument import function_end
from run import closure,run_one,write


def main():
    a=argparse.ArgumentParser();a.add_argument('--current',type=Path,required=True);a.add_argument('--historic-file',type=Path,required=True);a.add_argument('--api-receipt',type=Path,required=True);a.add_argument('--output',type=Path,required=True);args=a.parse_args()
    out=args.output.resolve();out.mkdir(exist_ok=False,parents=True)
    raw=args.historic_file.read_bytes();blob=hashlib.sha1(b'blob '+str(len(raw)).encode()+b'\0'+raw).hexdigest()
    receipt=json.loads(args.api_receipt.read_text());matches=[r for r in receipt if r.get('ref')=='f254bf2c890623d6d543ed3b874ea225117f1290' and r.get('path')=='firmware/src/app/ui_drum_voice.inc']
    if len(matches)!=1 or matches[0].get('sha')!=blob or matches[0]['content'].encode()!=raw:raise ValueError('authentic local historic blob does not match saved GitHub API receipt')
    source=out/'composed-source';shutil.copytree(args.current,source,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__'))
    path=source/'firmware/src/app/ui_drum_voice.inc';current=path.read_text();historic=raw.decode()
    oldstart=historic.index('static void dv_glyph(');oldbrace=historic.index('{',oldstart);oldend=function_end(historic,oldstart);body=historic[oldbrace:oldend]
    newstart=current.index('static UI_DRAW void dv_glyph(');newbrace=current.index('{',newstart);newend=function_end(current,newstart)
    path.write_text(current[:newbrace]+body+current[newend:])
    composition={'schema':'dnbox-authentic-historical-function-composition/1','baseProfileSha256':hashlib.sha256((args.current/'SOURCE-PROFILE.json').read_bytes()).hexdigest(),
        'baseSourceClosureSha256':closure(args.current)['sha256'],'historicalCommit':'f254bf2c890623d6d543ed3b874ea225117f1290','historicPath':'firmware/src/app/ui_drum_voice.inc',
        'historicGitBlobSha1':blob,'historicFileSha256':hashlib.sha256(raw).hexdigest(),'historicFunctionBodySha256':hashlib.sha256(body.encode()).hexdigest(),
        'method':'Exact authentic dv_glyph function BODY only; current signature/remaining source preserved. No claim this is a full historical source tree.',
        'apiReceiptSha256':hashlib.sha256(args.api_receipt.read_bytes()).hexdigest(),'purpose':'Real distinct-ref Tune/Pitch fork semantic negative, no fabricated duplicate parameter IDs.'}
    write(source/'COMPOSITION.json',composition);write(out/'composition.json',composition)
    report=run_one('historic-glyph',source,out)
    if 'checks' not in report:raise SystemExit(2)
    failures=report['checks']['2']['failures'];selected=[f for f in failures if f['reason']=='pitch-ratio-misrepresented-as-tune-fork']
    write(out/'semantic-result.json',{'sourceClosureSha256':report['sourceClosureSha256'],'oldExit':1 if selected else 0,'failures':selected,'rendererRun':report['run'],'composition':composition})
    print(json.dumps({'semanticOldExit':1 if selected else 0,'failures':len(selected),'rendererExit':report['run']['exit']}))
    raise SystemExit(1 if selected else 0)

if __name__=='__main__':main()
