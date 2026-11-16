#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Read-only GitHub API collector. Observed votes never authorize actions."""
from datetime import datetime, timezone
import json
import subprocess
from dnbox_inbox import REPOSITORY, evaluate

class ReadFailure(Exception):
    pass

def clock():
    return datetime.now(timezone.utc)

def github_read(endpoint):
    try:
        r=subprocess.run(['gh','api','--hostname','github.com','--method','GET',endpoint],capture_output=True,text=True,timeout=30,check=False)
    except (OSError,subprocess.TimeoutExpired) as error:
        raise ReadFailure('GitHub read route unavailable') from error
    if r.returncode:
        raise ReadFailure('GitHub API read failed; no partial inventory accepted')
    try:
        rows=json.loads(r.stdout)
    except ValueError as error:
        raise ReadFailure('Invalid GitHub JSON response') from error
    if not isinstance(rows,list):raise ReadFailure('Expected paginated GitHub list')
    return rows

def collect(read=github_read, now=clock):
    start_time=now();started=start_time.isoformat();ep='/repos/'+REPOSITORY+'/issues';reads=0
    def bounded_read(endpoint):
        nonlocal reads
        age=(now()-start_time).total_seconds()
        if age<0 or age>300 or reads>=256:raise ReadFailure('Collection freshness/request budget exceeded')
        reads+=1;rows=read(endpoint)
        age=(now()-start_time).total_seconds()
        if age<0 or age>300:raise ReadFailure('Collection freshness budget exceeded')
        return rows
    def pages(endpoint,query=''):
        observed=now().isoformat();result=[]
        for number in range(1,21):
            rows=bounded_read(endpoint+'?'+query+'per_page=100&page='+str(number))
            if not isinstance(rows,list):raise ReadFailure('Expected complete page list')
            result.append({'page':number,'status':200,'items':rows,'next_page':number+1 if rows else None})
            if not rows:return {'endpoint':endpoint,'read_status':'success','observed_at':observed,'pages':result}
        raise ReadFailure('Pagination bound reached before terminal empty page')
    issues=pages(ep,'state=open&');issues['query']={'state':'open'}
    reactions={}
    for page in issues['pages']:
        normalized=[]
        for issue in page['items']:
            if not isinstance(issue,dict):raise ReadFailure('Invalid issue row')
            row={key:issue.get(key) for key in ('id','number','title','state','updated_at')}
            if 'pull_request' in issue:row['pull_request']={}
            else:
                iid,number=issue.get('id'),issue.get('number')
                if type(iid) is not int or type(number) is not int or iid<=0 or number<=0 or str(iid) in reactions:
                    raise ReadFailure('Invalid or duplicate issue identity')
                envelope=pages(ep+'/'+str(number)+'/reactions')
                for rp in envelope['pages']:
                    safe=[]
                    for reaction in rp['items']:
                        if not isinstance(reaction,dict) or not isinstance(reaction.get('user'),dict):raise ReadFailure('Invalid reaction row')
                        safe.append({'id':reaction.get('id'),'content':reaction.get('content'),'user':{k:reaction['user'].get(k) for k in ('id','type')}})
                    rp['items']=safe
                reactions[str(iid)]=envelope
            normalized.append(row)
        page['items']=normalized
    snapshot={'schema':'dnbox-inbox-observation/2','repository':REPOSITORY,'observed_at':started,'issues':issues,'reactions':reactions}
    return snapshot,evaluate(snapshot,now())

def main():
    try:
        snapshot,report=collect()
    except (ReadFailure,KeyError,TypeError,ValueError) as error:
        report={'schema':'dnbox-inbox-report/2','status':'BLOCKED','items':[],'authorization':'NONE: read-only advisory observation','actions_performed':[],'reason':str(error)}
        snapshot=None
    print(json.dumps({'observation':snapshot,'report':report},ensure_ascii=False,indent=2))
    return 0 if report['status']=='OBSERVED' else 2

if __name__=='__main__':raise SystemExit(main())
