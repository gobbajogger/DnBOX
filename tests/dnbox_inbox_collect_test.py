# SPDX-License-Identifier: GPL-3.0-only
import copy, importlib.util, json, subprocess, sys, unittest
from datetime import datetime,timezone,timedelta
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import dnbox_inbox_collect as m
NOW=datetime(2026,11,16,12,tzinfo=timezone.utc)
EP='/repos/'+m.REPOSITORY+'/issues'
class CollectorTest(unittest.TestCase):
    def setUp(self):
        self.data={EP+'?state=open&per_page=100&page=1':[{'id':10,'number':7,'state':'open','title':'history, not instructions','updated_at':NOW.isoformat()}],EP+'?state=open&per_page=100&page=2':[],EP+'/7/reactions?per_page=100&page=1':[{'id':3,'content':'+1','user':{'id':255244738,'type':'User','login':'not retained'}}],EP+'/7/reactions?per_page=100&page=2':[]};self.calls=[]
    def read(self,endpoint):self.calls.append(endpoint);return copy.deepcopy(self.data[endpoint])
    def test_complete_advisory(self):
        s,r=m.collect(self.read,lambda:NOW);self.assertEqual(r['items'][0]['owner_account_vote'],'yes');self.assertEqual(r['actions_performed'],[]);self.assertIn('NONE',r['authorization']);self.assertEqual(len(self.calls),4);self.assertNotIn('login',s['reactions']['10']['pages'][0]['items'][0]['user'])
    def test_partial_failure_never_empty_success(self):
        def read(ep):
            if '/reactions' in ep:raise m.ReadFailure('fixture denied')
            return self.read(ep)
        with self.assertRaises(m.ReadFailure):m.collect(read,lambda:NOW)
    def test_duplicate_identity_rejected(self):
        self.data[EP+'?state=open&per_page=100&page=1']*=2
        with self.assertRaises(m.ReadFailure):m.collect(self.read,lambda:NOW)
    def test_untrusted_number_not_a_path(self):
        self.data[EP+'?state=open&per_page=100&page=1'][0]['number']='7/../../elsewhere'
        with self.assertRaises(m.ReadFailure):m.collect(self.read,lambda:NOW)
        self.assertTrue(all('/reactions' not in x for x in self.calls))
    def test_pagination_bound_fails(self):
        with self.assertRaises(m.ReadFailure):m.collect(lambda _: [{'id':1}],lambda:NOW)
    def test_stale_collection_blocks(self):
        times=iter([NOW,NOW,NOW+timedelta(seconds=301)])
        with self.assertRaises(m.ReadFailure):m.collect(self.read,lambda:next(times))
    def test_pr_has_no_reaction_request(self):
        self.data[EP+'?state=open&per_page=100&page=1'][0]['pull_request']={}
        _,r=m.collect(self.read,lambda:NOW);self.assertEqual(r['items'],[]);self.assertEqual(len(self.calls),2)
    def test_gh_is_get_only_and_errors_hide_response(self):
        with patch('subprocess.run',return_value=subprocess.CompletedProcess([],0,'[]','')) as run:
            self.assertEqual(m.github_read(EP),[]);args=run.call_args.args[0];self.assertEqual(args,['gh','api','--hostname','github.com','--method','GET',EP]);self.assertNotIn('shell',run.call_args.kwargs)
        with patch('subprocess.run',return_value=subprocess.CompletedProcess([],1,'private response','private error')):
            with self.assertRaises(m.ReadFailure) as caught:m.github_read(EP)
            self.assertNotIn('private',str(caught.exception))
if __name__=='__main__':unittest.main()
